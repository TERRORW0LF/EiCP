
#include <filesystem>
#include <thread>
#include "config.h"
#include "linear_algebra.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct Chunk
{
    std::vector<float> vertices{};
    std::vector<unsigned int> faces{};
};

#ifdef _WIN32

class File final
{
public:
    File(const std::filesystem::path &file_path)
    {
        HANDLE file_handle = CreateFileA(
            file_path.string().c_str(), GENERIC_READ, 0, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_READONLY | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
            nullptr);

        if (file_handle == INVALID_HANDLE_VALUE)
        {
            return;
        }

        LARGE_INTEGER size = LARGE_INTEGER{};
        if (!GetFileSizeEx(file_handle, &size))
        {
            close();
            return;
        }
        file_size = static_cast<size_t>(size.QuadPart);
    }
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
    HANDLE file_handle;
    size_t file_size;
};

struct Reader
{
    Reader(const File &file)
    {
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

    void read_block(size_t offset, size_t size, char *buffer)
    {
        overlapped.hEvent = read_handle;
        overlapped.Offset = static_cast<DWORD>(offset);
        overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
        bool success = ReadFile(file_handle, buffer, static_cast<DWORD>(size), nullptr, &overlapped);
        if (!success)
            std::cout << "Reading block failed" << std::endl;
    }

    void await_result()
    {
        bool success = GetOverlappedResult(read_handle, &overlapped, nullptr, TRUE);
        if (!success)
            std::cout << "Copying block failed" << std::endl;
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
    std::filesystem::path file_path(obj_path);
    if (file_path.empty())
        std::cout << "Incorrect file path" << std::endl;
    File file(file_path);
    if (!file)
        std::cout << "Can't open file" << std::endl;

    std::vector<Chunk> chunks = std::vector<Chunk>();

    int num_blocks = file.size() / 262144 + (file.size() % 262144 > 0);

    if (file.size() > 1048576)
    {
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

        for (int i = 0; i < tasks.size(); i++)
        {
            bool last = i == tasks.size() - 1;
            int begin = tasks[i];
            int end = last ? num_blocks : (tasks[i + 1] + 1);
            bool stop_at_eol = !last;
            Chunk *chunk = &chunks[i];

            threads.emplace_back(read_blocks, &file, begin, end, stop_at_eol, chunk);
        }
    }
    else
    {
        chunks.resize(1);
        read_blocks(&file, 0, num_blocks, false, &chunks.front());
    }
}

void read_blocks(File *file, int begin, int end, bool stop_at_eol, Chunk *chunk)
{
    bool begin_after_eol = begin > 0;
    bool reached_eof = false;
    Reader reader = Reader(*file);
    char *buffer[262144];
    size_t file_offset = begin * 262144;
}

#elif

/**
 * @brief Read an obj file and return the vertices and faces.
 *
 * @param obj_path The relative path to the obj file.
 * @returns A pair consisting of the vertices and faces vector.
 */
std::pair<std::vector<float>, std::vector<unsigned int>>
read_obj(const std::string &obj_path)
{
    const std::vector<float> vertices;
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