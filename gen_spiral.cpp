#include "vdspiral.h"
#include <Python.h>
#include <vector>


// Module method definitions
static PyObject* calc_traj_call(PyObject *self, PyObject *args, PyObject *keywds) {
    
    double gammabar = 42.5766;
    double grad_raster_time = 10.;

    int nitlv = 15;
    double res = 1.;
    double fov = 192.;
    double deltaz = 60.;
    double max_amp = 42.;
    double min_rise = 5.;
    int spiraltype = 3;
    double spiral_os = 1.;
    double vd_transition_begin = 0.18;
    double vd_transition_end = 0.25;

    static char *kwlist[] = {"nitlv", "res", "fov", "deltaz", "max_amp", "min_rise", "spiraltype", "spiral_os", "grad_raster_time", "vd_transition_begin", "vd_transition_end", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|idddddidddd", kwlist, &nitlv, &res, &fov, &deltaz, &max_amp, &min_rise, &spiraltype, &spiral_os, &grad_raster_time, &vd_transition_begin, &vd_transition_end))
        return NULL;

    if ((vd_transition_begin>=vd_transition_end) || (vd_transition_end>1.) || vd_transition_begin<0.)
        return NULL;     // wip: error messages

    std::vector<double> vFov(4, 0.);
    vFov[0] = fov*spiral_os; vFov[1] = vFov[0];
    vFov[2] = fov;           vFov[3] = vFov[2];

    std::vector<double> vRadius(4, 0.);
    vRadius[1] = vd_transition_begin; vRadius[2] = vd_transition_end; vRadius[3] = 1.;

    vdspiral spiralTraj;
    spiralTraj.prep(nitlv, res, vFov, vRadius, deltaz, max_amp, min_rise, vdspiral::eSpiralType(spiraltype), gammabar, grad_raster_time);
    
    std::vector<float> vfGx = spiralTraj.getGradX();
    std::vector<float> vfGy = spiralTraj.getGradY();
    std::vector<float> vfGz = spiralTraj.getGradZ();
    double amp_x = spiralTraj.getMaxAmplitudeX();
    double amp_y = spiralTraj.getMaxAmplitudeY();
    double amp_z = spiralTraj.getMaxAmplitudeZ();
    
    PyObject *ret = PyList_New(0);
    for (size_t k=0; k< vfGx.size(); ++k) {
        PyObject *coord = PyList_New(0);
        PyList_Append(coord, Py_BuildValue("f", amp_x*vfGx[k]));
        PyList_Append(coord, Py_BuildValue("f", amp_y*vfGy[k]));
        PyList_Append(coord, Py_BuildValue("f", amp_z*vfGz[k]));
        PyList_Append(ret, coord);
    }

    return ret;
}

// Method definition object for this extension, these argumens mean:
// ml_name: The name of the method
// ml_meth: Function pointer to the method implementation
// ml_flags: Flags indicating special features of this method, such as
//          accepting arguments, accepting keyword arguments, being a
//          class method, or being a static method of a class.
// ml_doc:  Contents of this method's docstring
static PyMethodDef traj_methods[] = { 
    {   
    "calc_traj", (PyCFunction)(void(*)(void))calc_traj_call, METH_VARARGS | METH_KEYWORDS,
    "Calculate spiral gradient trajectory from input parameters.\n\\
    Parameters\n\\
    ----------\n\\
        nitlv : int\n\\
            number of spiral interleaves (default: 15)\n\\
        res : float\n\\
            target resolution (default: 1 mm)\n\\
        fov : float\n\\
            target field-of-view (default: 192 mm)\n\\
        max_amp : float\n\\
            maximum gradient amplitude (default: 42 mT/m)\n\\
        min_rise : float\n\\
            minimum gradient risetime (default: 5 us/(mT/m))\n\\
        spiraltype: int\n\\
            1: spiral out, 2: spiral in, 3: double spiral (default), 4: spiral out/in (same path), \\
            5: spiral in out, 6: double spiral reversed, 7: Laterally Oscillating Trajectory for Undersampling Slices (LOTUS), \n\\
        spiral_os: float\n\\
            spiral oversampling (default: 1)\n\\
        vd_transition_begin: float\n\\
            beginning of variable density transition region (default: 0.18)\n\\
        vd_transition_end: float\n\\
            end of variable density transition region (default: 0.25)\n\\
        deltaz : float\n\\
            Slice separation of two multiband slices in z-direction (default: 60 mm)\n\\
            Only used for LOTUS spiral type.\n\\
    Returns\n\\
    ----------\n\\
        List of lists of calculated Gx & Gy values [mT/m]\n\\
        Shape: [samples, 2]\n"
    },
    {NULL, NULL, 0, NULL}
};

// Module definition
// The arguments of this structure tell Python what to call your extension,
// what it's methods are and where to look for it's method definitions
static struct PyModuleDef traj_definition = { 
    PyModuleDef_HEAD_INIT,
    "spiraltraj",
    "A Python module that calculates spiral gradient trajectories.",
    -1, 
    traj_methods
};

// Module initialization
// Python calls this function when importing your extension. It is important
// that this function is named PyInit_[[your_module_name]] exactly, and matches
// the name keyword argument in setup.py's setup() call.
PyMODINIT_FUNC PyInit_spiraltraj(void) {
    Py_Initialize();
    return PyModule_Create(&traj_definition);
}
