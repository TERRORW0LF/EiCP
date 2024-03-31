#include "obj_reader.h"
#include <cassert>

/**
 * @param text The text to trim.
 *
 * @brief Trims a text in place of its whitespace at the start.
 */
constexpr void trim_left(std::string_view &text) noexcept
{
    size_t i = 0;
    while (i < text.size())
    {
        if (text[i] == ' ')
            i++;
        else
            break;
    }
    text.remove_prefix(i);
}

// File and Reader classes are OS dependant.
// We currently only support Windows.
#ifdef _WIN32
struct Chunk
{
    std::vector<float> vertices{};
    std::vector<unsigned int> faces{};
};

class File final
{
public:
    File(const std::filesystem::path &file_path)
    {
        // Create a file with read permissions and allow multiple readers
        // to read at once.
        file_handle = CreateFileA(
            file_path.string().c_str(), GENERIC_READ, 0, nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_READONLY | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
            nullptr);

        if (file_handle == INVALID_HANDLE_VALUE)
        {
            std::cout << "Invalid file" << std::endl;
            return;
        }

        // Get the file size
        LARGE_INTEGER size = LARGE_INTEGER{};
        if (!GetFileSizeEx(file_handle, &size))
        {
            close();
            return;
        }
        file_size = static_cast<size_t>(size.QuadPart);
    }
    // Override constructors and destructors
    File(const File &) = delete;
    File &operator=(const File &) = delete;
    File(File &&) = delete;
    File &operator=(File &&) = delete;
    ~File() noexcept { close(); }

    explicit operator bool() const noexcept { return file_handle != nullptr && file_handle != INVALID_HANDLE_VALUE; }
    HANDLE handle() const noexcept { return file_handle; }
    size_t size() const noexcept { return file_size; }

private:
    void close() noexcept
    {
        if (file_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(file_handle);
            file_handle = INVALID_HANDLE_VALUE;
        }
    }
    HANDLE file_handle{};
    size_t file_size{};
};

struct Reader
{
    Reader(const File &file)
    {
        // Create a read handle.
        read_handle = CreateEventA(nullptr, FALSE, FALSE, nullptr);
        if (read_handle == INVALID_HANDLE_VALUE)
            std::cout << "Cant't read blocks" << std::endl;

        file_handle = file.handle();
    }

    ~Reader() noexcept
    {
        if (read_handle != INVALID_HANDLE_VALUE)
            CloseHandle(read_handle);
    }

    /**
     * @param offset The file offset to start reading at.
     * @param size The amount of bytes to read.
     * @param buffer The buffer to write the result into.
     *
     * @brief Asynchronously read a part of the file connected to the reader.
     */
    void read_block(size_t offset, size_t size, char *buffer)
    {
        overlapped.hEvent = read_handle;
        overlapped.Offset = static_cast<DWORD>(offset);
        overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
        bool success = ReadFile(file_handle, buffer, static_cast<DWORD>(size), nullptr, &overlapped);
        if (!success)
        {
            DWORD error = GetLastError();
            // When IO is pending we just have to wait.
            if (error != ERROR_IO_PENDING)
            {
                std::cout << "Reading block failed: " << GetLastError() << std::endl;
            }
        }
    }

    /**
     * @returns The number of bytes read.
     *
     * @brief Waits for the last read to finish.
     */
    size_t await_result()
    {
        DWORD bytes_read = DWORD{};
        bool success = GetOverlappedResult(read_handle, &overlapped, &bytes_read, TRUE);
        if (!success)
            std::cout << "Copying block failed" << std::endl;
        return bytes_read;
    }

private:
    HANDLE read_handle{};
    HANDLE file_handle{};
    OVERLAPPED overlapped{};
};

/**
 * @brief Read an obj file and return the vertices and faces.
 *
 * @param obj_path The relative path to the obj file.
 * @returns A pair consisting of the vertices and faces vector.
 */
std::pair<std::vector<float>, std::vector<unsigned int>> read_obj(const std::string &obj_path)
{
    // Get the full file path from the relative given path.
    std::filesystem::path rel_path(obj_path);
    std::filesystem::path file_path = std::filesystem::absolute(rel_path);
    if (file_path.empty())
        std::cout << "Incorrect file path" << std::endl;
    File file(file_path);
    if (!file)
        std::cout << "Can't open file" << std::endl;

    // Prepare output vector and blocks.
    std::vector<Chunk> chunks = std::vector<Chunk>();

    int num_blocks = file.size() / block_size + (file.size() % block_size > 0);

    // Use multithreading to speed up reading.
    if (file.size() > thread_threshhold)
    {
        // Allocate blocks to threads.
        unsigned int num_threads = std::thread::hardware_concurrency();
        int blocks_per_thread = num_blocks / num_threads;
        int blocks_remain = num_blocks - (num_threads * blocks_per_thread);

        std::vector<int> tasks = std::vector<int>();
        tasks.reserve(num_threads);
        int block = 0;
        while (block < num_blocks)
        {
            tasks.push_back(block);
            block += blocks_per_thread;
            // Add an extra block per thread if there's still blocks remaining.
            if (blocks_remain > 0)
            {
                block++;
                blocks_remain--;
            }
        }

        int num_tasks = tasks.size();
        std::vector<std::thread> threads = std::vector<std::thread>{};
        chunks.resize(num_tasks);
        threads.reserve(num_tasks);

        // Create threads with corresponding blocks.
        for (int i = 0; i < tasks.size(); i++)
        {
            bool last = i == tasks.size() - 1;
            int begin = tasks[i];
            int end = last ? num_blocks : (tasks[i + 1] + 1);
            bool stop_at_eol = !last;
            Chunk *chunk = &chunks[i];

            threads.emplace_back(read_blocks, &file, begin, end, stop_at_eol, chunk);
        }

        // Wait for all threads to finish.
        for (std::thread &thread : threads)
            thread.join();
    }
    else
    {
        chunks.resize(1);
        read_blocks(&file, 0, num_blocks, false, &chunks.front());
    }

    // Merge the result of all threads.
    return merge(chunks);
}

void read_blocks(File *file, int begin, int end, bool stop_at_eol, Chunk *chunk)
{
    // We always read full lines, therefore all block except the first one
    // will skip the incomplete first line.
    bool begin_after_eol = begin > 0;
    bool reached_eof = false;

    // Initialize reader to read chunks of data of the file.
    Reader reader = Reader(*file);
    std::string_view line = std::string_view();
    std::string_view text = std::string_view();
    size_t buffer_size = block_size + max_line;
    auto buffer1 = std::unique_ptr<char>(static_cast<char *>(malloc(buffer_size)));
    auto buffer2 = std::unique_ptr<char>(static_cast<char *>(malloc(buffer_size)));
    auto front_buffer = buffer1.get();
    auto back_buffer = buffer2.get();
    size_t file_offset = begin * block_size;

    // Read the first block and save it for consumption.
    reader.read_block(file_offset, block_size, front_buffer + max_line);
    size_t bytes_read = reader.await_result();
    reached_eof = bytes_read < block_size;
    text = std::string_view(front_buffer + max_line, bytes_read);

    // If this is not the first overall block, throw away incomplete first line.
    if (begin_after_eol)
    {
        const char *eol = static_cast<const char *>(memchr(text.data(), '\n', max_line));
        size_t length = static_cast<size_t>(eol - text.data());
        text.remove_prefix(length + 1);
    }

    // Iterate over the rest of the blocks.
    for (int i = begin; i < end; i++)
    {
        // The size of the last part of a block not in a full line.
        size_t remainder = size_t{};
        bool last_block = (i == end - 1) || reached_eof;

        if (!last_block)
        {
            // If this is not the last block, prepare the next block in advance.
            file_offset = (i + 1) * block_size;
            reader.read_block(file_offset, block_size, back_buffer + max_line);
        }
        else if (stop_at_eol)
        {
            // The last block is actually the frist block of the next chunk.
            // We therefore only want to read the first line, which the next chunk will
            // throw away.
            const char *eol = static_cast<const char *>(memchr(text.data(), '\n', max_line));
            if (eol != nullptr)
            {
                size_t line_length = static_cast<size_t>(eol - text.data());
                line = text.substr(0, line_length);
                if (line.ends_with('\r'))
                {
                    line.remove_suffix(1);
                }

                consume_line(line, chunk);
            }
            return;
        }

        while (!text.empty())
        {
            const char *eol = static_cast<const char *>(memchr(text.data(), '\n', std::min(max_line, text.size())));
            if (eol != nullptr)
            {
                assert(eol > text.data());

                size_t line_length = static_cast<size_t>(eol - text.data());
                line = text.substr(0, line_length);
                if (line.ends_with('\r'))
                {
                    line.remove_suffix(1);
                }

                assert(text.length() >= line_length + 1);
                text.remove_prefix(line_length + 1);
            }
            else
            {
                // This should only happen at the end of the file.
                if (last_block)
                {
                    line = text;
                    consume_line(line, chunk);
                }
                else
                {
                    // We have reached the end of the block. Add incomplete line to
                    // the start of the next buffer.
                    remainder = text.size();
                    memcpy(back_buffer + max_line - remainder, text.data(), remainder);
                }
                // Prepare for the next block.
                text = {};
                break;
            }

            consume_line(line, chunk);
        }

        if (!last_block)
        {
            // Wait for the read process started at the beginning
            // of the loop to finish.
            size_t bytes_read = reader.await_result();
            reached_eof = bytes_read < block_size;

            // Put the new block in the working buffer.
            // Prepare next block with the filled incompleted line of the last buffer.
            std::swap(front_buffer, back_buffer);
            text = std::string_view(front_buffer + max_line - remainder, bytes_read + remainder);
        }
        else if (reached_eof)
            break;
    }
}

void consume_line(std::string_view line, Chunk *chunk)
{
    // Ignore line if empty.
    if (line.empty())
        return;

    // We currently only need to support vertices (v) and faces (f)
    switch (line.front())
    {
    case 'v':
        line.remove_prefix(1);

        // Our vertices have only position and no extra information.
        for (int i = 0; i < 3 && !line.empty(); i++)
        {
            trim_left(line);
            float value = float();
            auto [ptr, _] = std::from_chars(line.data(), line.data() + line.size(), value);
            chunk->vertices.push_back(value);
            size_t length = static_cast<size_t>(ptr - line.data());
            line.remove_prefix(length);
        }
        break;
    case 'f':
        line.remove_prefix(1);
        for (int i = 0; i < 3 && !line.empty(); i++)
        {
            trim_left(line);
            unsigned int value = 0;
            auto [ptr, _] = std::from_chars(line.data(), line.data() + line.size(), value);
            chunk->faces.push_back(value - 1);
            size_t length = static_cast<size_t>(ptr - line.data());
            line.remove_prefix(length);
        }
        break;
    }
}

/**
 * @param chunks The individual results to combine.
 * @returns The combined chunks.
 *
 * @brief Combines chunks such that the structure of the mesh is preserved.
 */
std::pair<std::vector<float>, std::vector<unsigned int>> merge(std::vector<Chunk> chunks)
{
    size_t vertices_length = size_t{};
    size_t faces_length = size_t{};
    std::vector<float> vertices;
    std::vector<unsigned int> faces;

    for (Chunk chunk : chunks)
    {
        vertices_length += chunk.vertices.size();
        faces_length += chunk.faces.size();
    }

    vertices.resize(vertices_length);
    faces.resize(faces_length);

    vertices_length = 0;
    faces_length = 0;

    for (Chunk chunk : chunks)
    {
        // We can just add the chunk to the end bc blocks in a chunk and the chunks
        // itself are in ascending order.
        std::move(chunk.vertices.begin(), chunk.vertices.end(), vertices.begin() + vertices_length);
        std::move(chunk.faces.begin(), chunk.faces.end(), faces.begin() + faces_length);
        vertices_length += chunk.vertices.size();
        faces_length += chunk.faces.size();
    }

    return std::make_pair(vertices, faces);
}

#else

/**
 * @brief Read an obj file and return the vertices and faces.
 *
 * @param obj_path The relative path to the obj file.
 * @returns A pair consisting of the vertices and faces vector.
 */
std::pair<std::vector<float>, std::vector<unsigned int>> read_obj(const std::string &obj_path)
{
    std::vector<float> vertices;
    std::vector<unsigned int> faces;

    // Setup streams for reading obj file.
    std::ifstream file;
    std::string line;

    // Try to open the shader file. Print warning if that failed.
    file.open(obj_path);
    if (!file.is_open())
    {
        std::cout << "Unable to open obj file." << std::endl;
    }

    // Read file and convert it into a char array.
    // Get a line
    while (std::getline(file, line))
    {
        // Split line by spaces
        std::stringstream line_stream(line);
        std::string prefix;
        float values[3];
        line_stream >> prefix;
        // check prefix
        if (prefix == "v")
        {
            // Read a vertix
            line_stream >> values[0] >> values[1] >> values[2];
            vertices.insert(vertices.end(), values, values + 3);
        }
        else if (prefix == "f")
        {
            // read a face. Convert to 0 based array.
            line_stream >> values[0] >> values[1] >> values[2];
            values[0] -= 1;
            values[1] -= 1;
            values[2] -= 1;
            faces.insert(faces.end(), values, values + 3);
        }
    }

    return std::make_pair(vertices, faces);
}
#endif