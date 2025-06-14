import matplotlib.pyplot as plt
import numpy as np
import math
numberOfSamples = 50
def normalize(arr):
    #return arr
    l = 0
    for i in arr:
        l += i ** 2
    l **= 0.5
    return arr / l
def sin(x):
    #print(x, np.sin(x))
    sx = -1 if x < 0 else 1
    nx = abs(x) / (2 * np.pi)
    nx = nx - math.floor(nx)
    #v = 20.6 * nx * (nx - 0.5) * (nx - 1) * sx
    o = nx if nx < 0.5 else -(nx-1)
    v = 16 * (nx-0.5) * o * sx
    #print(v)
    return v
    pass
    #return np.sin(x)
def cos(x):
    return sin(x + np.pi / 2)
    return np.cos(x)
def arccos(x):
    return np.arccos(x)

def f(rayNumber):
    goldenRation = (1.0 + np.sqrt(5)) / 2
    x = rayNumber / goldenRation
    x = x - math.floor(x)
    y = rayNumber / (numberOfSamples - 1)
    theta = 2 * np.pi * x
    phi = arccos(1.0 - 2.0 * y)
    sp = np.array([
        cos(theta)*sin(phi),
        abs(1.0-2.0*y)+0.01,
        sin(theta)*sin(phi) 
    ],dtype=np.float32)
    v0,v1,v2 = np.array([1,0,0],dtype=np.float32),np.array([-1,0,0],dtype=np.float32),np.array([1,1,-1],dtype=np.float32)
    v0v1 = v1 - v0
    v0v2 = v2 - v0
    faceNormal = normalize(np.cross(v0v1,v0v2))
    A = faceNormal
    B = normalize(v0v1)
    C = np.cross(A,B)
    ax.plot3D([-B[0],B[0]], [-B[2],B[2]], [-B[1],B[1]], 'red')
    ax.plot3D([-C[0],C[0]], [-C[2],C[2]], [-C[1],C[1]], 'blue')
    ax.plot3D([-A[0],A[0]], [-A[2],A[2]], [-A[1],A[1]], 'green')
    lst = [v0,v1,v2]
    for i in lst:
        ax.scatter([i[0]], [i[2]], [i[1]], color='yellow', marker='^', s=50)
    #return sp
    return normalize(np.array([
        A[0] * sp[1] + B[0] * sp[0] + C[0] * sp[2],
        A[1] * sp[1] + B[1] * sp[0] + C[1] * sp[2],
        A[2] * sp[1] + B[2] * sp[0] + C[2] * sp[2],
    ],dtype=np.float32))
    pass
    '''
    using std::cos; using std::sin;
    const real goldenRatio = (1.f + sqrt(5.f)) / 2.f;
    real x = (real)rayNumber / goldenRatio;
    x = x - (int)x;
    real y = (real)rayNumber / (real)(numberOfSamples-1) / 2.01f;///2.f means only generate for hemisphere
    real theta = 2.f * PI * x;
    real phi = std::acos(1.f-2.f*y);
    Vec3 standardPos = Vec3(
        cos(theta)*sin(phi),
        (1.f-2.f*y),
        sin(theta)*sin(phi)        
    );
    Vec3 A = faceNormal;
    Vec3 B = (facePtr->vertexList[1] - facePtr->vertexList[0]).normalize();
    Vec3 C = cross(A,B).normalize();
    return Vec3(
        dot(B, standardPos),
        dot(A, standardPos),
        dot(C, standardPos)
    ).normalize();
    '''
fig = plt.figure()
ax = plt.axes(projection='3d')
ax.set_xlim3d(-1, 1)    # X-axis will range from 0 to 10
ax.set_ylim3d(-1, 1)    # Y-axis will range from 0 to 20
ax.set_zlim3d(-1, 1)    # Z-axis will range from 0 to 50
x,y,z = [],[],[]
for s in range(numberOfSamples):
    a,b,c = f(s)
    x.append(a); y.append(b); z.append(c)
    x.append(0); y.append(0); z.append(0)
ax.plot3D(x, z, y, 'purple')
plt.show()
#some small change