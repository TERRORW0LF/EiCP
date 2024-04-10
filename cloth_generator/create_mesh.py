import numpy as np
import random

NUM_ROWS=200
NUM_COLS=200
HEIGHT=1
WIDTH=1
QUADS = False
NOISE=True

vertices = []
rows = np.linspace(-HEIGHT/2, HEIGHT/2, NUM_ROWS)
cols = np.linspace(-WIDTH/2, WIDTH/2, NUM_COLS)
for y in rows:
    for x in cols:
        vertices.append((x, y))

vertices_with_index = list(enumerate(vertices))

quads = []
tris = []
for row in range(NUM_ROWS - 1):
    for col in range(NUM_COLS - 1):
        v1 = row * NUM_COLS + col
        v2 = (row+1) * NUM_COLS + col
        v3 = (row+1) * NUM_COLS + col +1
        v4 = row * NUM_COLS + col +1
        quads.append((v1,v2,v3,v4))
    
        tris.append((v1,v2,v4))
        tris.append((v2,v3,v4))


with open("subdivided_plane.obj", "w") as obj_file:
    # Write the vertices to the OBJ file
    for v in vertices:
        obj_file.write(f"v {v[0]} {v[1]} {random.uniform(-0.01, 0.01) if NOISE else 0}\n")

    if QUADS:
        for f in quads:
            obj_file.write(f"f {f[0]+1} {f[1]+1} {f[2]+1} {f[3]+1}\n")
    else:
        for f in tris:
            obj_file.write(f"f {f[0]+1} {f[1]+1} {f[2]+1}\n")
