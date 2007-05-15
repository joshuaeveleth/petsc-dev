static char help[] = "This example solves the Stokes problem.\n\n";

/*
  -\Delta u - \nabla p = f
        \nabla \cdot u = 0
*/

#include <petscda.h>
#include <petscmesh.h>
#include <petscdmmg.h>

using ALE::Obj;
typedef enum {RUN_FULL, RUN_TEST, RUN_MESH} RunType;
typedef enum {NEUMANN, DIRICHLET} BCType;
typedef enum {ASSEMBLY_FULL, ASSEMBLY_STORED, ASSEMBLY_CALCULATED} AssemblyType;
typedef union {SectionReal section; Vec vec;} ExactSolType;

typedef struct {
  PetscScalar u, v, p;
} Field2D;

typedef struct {
  PetscScalar u, v, w, p;
} Field3D;

typedef struct {
  PetscInt      debug;                                        // The debugging level
  RunType       run;                                          // The run type
  PetscInt      dim;                                          // The topological mesh dimension
  PetscTruth    structured;                                   // Use a structured mesh
  PetscTruth    generateMesh;                                 // Generate the unstructure mesh
  PetscTruth    interpolate;                                  // Generate intermediate mesh elements
  PetscReal     refinementLimit;                              // The largest allowable cell volume
  char          baseFilename[2048];                           // The base filename for mesh files
  void        (*func)(const double [], double []);            // The function to project
  BCType        bcType;                                       // The type of boundary conditions
  void        (*exactFunc)(const double [], double []);       // The exact solution function
  ExactSolType  exactSol;                                     // The discrete exact solution
  AssemblyType  operatorAssembly;                             // The type of operator assembly 
  double (*integrateV)(const double *, const double *, const int, double (*)(const double *)); // Basis functional application
  double (*integrateP)(const double *, const double *, const int, double (*)(const double *)); // Basis functional application
} Options;

#include "stokes_quadrature.h"

void constant(const double x[], double f[]) {
  f[0] = -3.0;
  f[1] = -3.0;
  f[2] =  0.0;
}

void quadratic_2d(const double x[], double f[]) {
  f[0] = x[0]*x[0] - 2.0*x[0]*x[1];
  f[1] = x[1]*x[1] - 2.0*x[0]*x[1];
  f[2] = x[0] + x[1] - 1.0;
}

void quadratic_3d(const double x[], double f[]) {
  f[0] = x[0]*x[0] - x[0]*x[1] - x[0]*x[2];
  f[1] = x[1]*x[1] - x[0]*x[1] - x[1]*x[2];
  f[2] = x[2]*x[2] - x[0]*x[2] - x[1]*x[2];
  f[3] = x[0] + x[1] + x[2] - 1.5;
}

void cos_2d(const double x[], double f[]) {
  f[0] = cos(2.0*PETSC_PI*x[0]);
  f[1] = cos(2.0*PETSC_PI*x[1]);
  f[2] = cos(2.0*PETSC_PI*x[0]);
}

void cos_3d(const double x[], double f[]) {
  f[0] = cos(2.0*PETSC_PI*x[0]);
  f[1] = cos(2.0*PETSC_PI*x[1]);
  f[2] = cos(2.0*PETSC_PI*x[2]);
  f[3] = cos(2.0*PETSC_PI*x[0]);
}

#undef __FUNCT__
#define __FUNCT__ "ProcessOptions"
PetscErrorCode ProcessOptions(MPI_Comm comm, Options *options)
{
  const char    *runTypes[3] = {"full", "test", "mesh"};
  const char    *bcTypes[2]  = {"neumann", "dirichlet"};
  const char    *asTypes[4]  = {"full", "stored", "calculated"};
  ostringstream  filename;
  PetscInt       run, bc, as;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  options->debug            = 0;
  options->run              = RUN_FULL;
  options->dim              = 2;
  options->structured       = PETSC_TRUE;
  options->generateMesh     = PETSC_FALSE;
  options->interpolate      = PETSC_TRUE;
  options->refinementLimit  = 0.0;
  options->bcType           = DIRICHLET;
  options->operatorAssembly = ASSEMBLY_FULL;

  ierr = PetscOptionsBegin(comm, "", "Stokes Problem Options", "DMMG");CHKERRQ(ierr);
    ierr = PetscOptionsInt("-debug", "The debugging level", "stokes.cxx", options->debug, &options->debug, PETSC_NULL);CHKERRQ(ierr);
    run = options->run;
    ierr = PetscOptionsEList("-run", "The run type", "stokes.cxx", runTypes, 3, runTypes[options->run], &run, PETSC_NULL);CHKERRQ(ierr);
    options->run = (RunType) run;
    ierr = PetscOptionsInt("-dim", "The topological mesh dimension", "stokes.cxx", options->dim, &options->dim, PETSC_NULL);CHKERRQ(ierr);
    ierr = PetscOptionsTruth("-structured", "Use a structured mesh", "stokes.cxx", options->structured, &options->structured, PETSC_NULL);CHKERRQ(ierr);
    ierr = PetscOptionsTruth("-generate", "Generate the unstructured mesh", "stokes.cxx", options->generateMesh, &options->generateMesh, PETSC_NULL);CHKERRQ(ierr);
    ierr = PetscOptionsTruth("-interpolate", "Generate intermediate mesh elements", "stokes.cxx", options->interpolate, &options->interpolate, PETSC_NULL);CHKERRQ(ierr);
    ierr = PetscOptionsReal("-refinement_limit", "The largest allowable cell volume", "stokes.cxx", options->refinementLimit, &options->refinementLimit, PETSC_NULL);CHKERRQ(ierr);
    filename << "data/stokes_" << options->dim <<"d";
    ierr = PetscStrcpy(options->baseFilename, filename.str().c_str());CHKERRQ(ierr);
    ierr = PetscOptionsString("-base_filename", "The base filename for mesh files", "stokes.cxx", options->baseFilename, options->baseFilename, 2048, PETSC_NULL);CHKERRQ(ierr);
    bc = options->bcType;
    ierr = PetscOptionsEList("-bc_type","Type of boundary condition","stokes.cxx",bcTypes,2,bcTypes[options->bcType],&bc,PETSC_NULL);CHKERRQ(ierr);
    options->bcType = (BCType) bc;
    as = options->operatorAssembly;
    ierr = PetscOptionsEList("-assembly_type","Type of operator assembly","stokes.cxx",asTypes,3,asTypes[options->operatorAssembly],&as,PETSC_NULL);CHKERRQ(ierr);
    options->operatorAssembly = (AssemblyType) as;
  ierr = PetscOptionsEnd();

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "CreatePartition"
// Creates a field whose value is the processor rank on each element
PetscErrorCode CreatePartition(Mesh mesh, SectionInt *partition)
{
  Obj<ALE::Mesh> m;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = MeshGetCellSectionInt(mesh, 1, partition);CHKERRQ(ierr);
  const Obj<ALE::Mesh::label_sequence>&     cells = m->heightStratum(0);
  const ALE::Mesh::label_sequence::iterator end   = cells->end();
  const int                                 rank  = m->commRank();

  for(ALE::Mesh::label_sequence::iterator c_iter = cells->begin(); c_iter != end; ++c_iter) {
    ierr = SectionIntUpdate(*partition, *c_iter, &rank);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "ViewSection"
PetscErrorCode ViewSection(Mesh mesh, SectionReal section, const char filename[])
{
  MPI_Comm       comm;
  SectionInt     partition;
  PetscViewer    viewer;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscObjectGetComm((PetscObject) mesh, &comm);CHKERRQ(ierr);
  ierr = PetscViewerCreate(comm, &viewer);CHKERRQ(ierr);
  ierr = PetscViewerSetType(viewer, PETSC_VIEWER_ASCII);CHKERRQ(ierr);
  ierr = PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_VTK);CHKERRQ(ierr);
  ierr = PetscViewerFileSetName(viewer, filename);CHKERRQ(ierr);
  ierr = MeshView(mesh, viewer);CHKERRQ(ierr);
  ierr = SectionRealView(section, viewer);CHKERRQ(ierr);
  ierr = CreatePartition(mesh, &partition);CHKERRQ(ierr);
  ierr = PetscViewerPushFormat(viewer, PETSC_VIEWER_ASCII_VTK_CELL);CHKERRQ(ierr);
  ierr = SectionIntView(partition, viewer);CHKERRQ(ierr);
  ierr = SectionIntDestroy(partition);CHKERRQ(ierr);
  ierr = PetscViewerPopFormat(viewer);CHKERRQ(ierr);
  ierr = PetscViewerDestroy(viewer);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "ViewMesh"
PetscErrorCode ViewMesh(Mesh mesh, const char filename[])
{
  MPI_Comm       comm;
  SectionInt     partition;
  PetscViewer    viewer;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscObjectGetComm((PetscObject) mesh, &comm);CHKERRQ(ierr);
  ierr = PetscViewerCreate(comm, &viewer);CHKERRQ(ierr);
  ierr = PetscViewerSetType(viewer, PETSC_VIEWER_ASCII);CHKERRQ(ierr);
  ierr = PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_VTK);CHKERRQ(ierr);
  ierr = PetscViewerFileSetName(viewer, filename);CHKERRQ(ierr);
  ierr = MeshView(mesh, viewer);CHKERRQ(ierr);
  ierr = CreatePartition(mesh, &partition);CHKERRQ(ierr);
  ierr = PetscViewerPushFormat(viewer, PETSC_VIEWER_ASCII_VTK_CELL);CHKERRQ(ierr);
  ierr = SectionIntView(partition, viewer);CHKERRQ(ierr);
  ierr = PetscViewerPopFormat(viewer);CHKERRQ(ierr);
  ierr = SectionIntDestroy(partition);CHKERRQ(ierr);
  ierr = PetscViewerDestroy(viewer);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "CreateMesh"
PetscErrorCode CreateMesh(MPI_Comm comm, DM *dm, Options *options)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->structured) {
    DA da;

    if (options->dim == 2) {
      ierr = DACreate2d(comm, DA_NONPERIODIC, DA_STENCIL_BOX, -3, -3, PETSC_DECIDE, PETSC_DECIDE, 3, 1, PETSC_NULL, PETSC_NULL, &da);CHKERRQ(ierr);
    } else if (options->dim == 3) {
      ierr = DACreate3d(comm, DA_NONPERIODIC, DA_STENCIL_BOX, -3, -3, -3, PETSC_DECIDE, PETSC_DECIDE, PETSC_DECIDE, 4, 1, PETSC_NULL, PETSC_NULL, PETSC_NULL, &da);CHKERRQ(ierr);
    } else {
      SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", options->dim);
    }
    ierr = DASetUniformCoordinates(da, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);CHKERRQ(ierr);
    *dm = (DM) da;
  } else {
    Mesh        mesh;
    PetscTruth  view;
    PetscMPIInt size;

    if (options->generateMesh) {
      Mesh boundary;

      ierr = MeshCreate(comm, &boundary);CHKERRQ(ierr);
      if (options->dim == 2) {
        double lower[2] = {0.0, 0.0};
        double upper[2] = {1.0, 1.0};
        int    edges[2] = {2, 2};

        Obj<ALE::Mesh> mB = ALE::MeshBuilder::createSquareBoundary(comm, lower, upper, edges, options->debug);
        ierr = MeshSetMesh(boundary, mB);CHKERRQ(ierr);
      } else if (options->dim == 3) {
        double lower[3] = {0.0, 0.0, 0.0};
        double upper[3] = {1.0, 1.0, 1.0};
        int    faces[3] = {1, 1, 1};

        Obj<ALE::Mesh> mB = ALE::MeshBuilder::createCubeBoundary(comm, lower, upper, faces, options->debug);
        ierr = MeshSetMesh(boundary, mB);CHKERRQ(ierr);
      } else {
        SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", options->dim);
      }
      ierr = MeshGenerate(boundary, options->interpolate, &mesh);CHKERRQ(ierr);
      ierr = MeshDestroy(boundary);CHKERRQ(ierr);
    } else {
      std::string baseFilename(options->baseFilename);
      std::string coordFile = baseFilename+".nodes";
      std::string adjFile   = baseFilename+".lcon";

      ierr = MeshCreatePCICE(comm, options->dim, coordFile.c_str(), adjFile.c_str(), options->interpolate, PETSC_NULL, &mesh);CHKERRQ(ierr);
    }
    ierr = MPI_Comm_size(comm, &size);CHKERRQ(ierr);
    if (size > 1) {
      Mesh parallelMesh;

      ierr = MeshDistribute(mesh, PETSC_NULL, &parallelMesh);CHKERRQ(ierr);
      ierr = MeshDestroy(mesh);CHKERRQ(ierr);
      mesh = parallelMesh;
    }
    if (options->refinementLimit > 0.0) {
      Mesh refinedMesh;

      ierr = MeshRefine(mesh, options->refinementLimit, options->interpolate, &refinedMesh);CHKERRQ(ierr);
      ierr = MeshDestroy(mesh);CHKERRQ(ierr);
      mesh = refinedMesh;
    }
    if (options->bcType == DIRICHLET) {
      Obj<ALE::Mesh> m;

      ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
      m->markBoundaryCells("marker");
    }
    ierr = PetscOptionsHasName(PETSC_NULL, "-mesh_view_vtk", &view);CHKERRQ(ierr);
    if (view) {ierr = ViewMesh(mesh, "stokes.vtk");CHKERRQ(ierr);}
    ierr = PetscOptionsHasName(PETSC_NULL, "-mesh_view", &view);CHKERRQ(ierr);
    if (view) {
      Obj<ALE::Mesh> m;
      ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
      m->view("Mesh");
    }
    *dm = (DM) mesh;
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DestroyMesh"
PetscErrorCode DestroyMesh(DM dm, Options *options)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->structured) {
    ierr = DADestroy((DA) dm);CHKERRQ(ierr);
  } else {
    ierr = MeshDestroy((Mesh) dm);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DestroyExactSolution"
PetscErrorCode DestroyExactSolution(ExactSolType sol, Options *options)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->structured) {
    ierr = VecDestroy(sol.vec);CHKERRQ(ierr);
  } else {
    ierr = SectionRealDestroy(sol.section);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "Function_Structured_2d"
PetscErrorCode Function_Structured_2d(DALocalInfo *info, Field2D *x[], Field2D *f[], void *ctx)
{
  Options       *options = (Options *) ctx;
  void         (*func)(const double [], double[]) = options->func;
  DA             coordDA;
  Vec            coordinates;
  DACoor2d     **coords;
  PetscInt       i, j;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DAGetCoordinateDA(info->da, &coordDA);CHKERRQ(ierr);
  ierr = DAGetCoordinates(info->da, &coordinates);CHKERRQ(ierr);
  ierr = DAVecGetArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  for(j = info->ys; j < info->ys+info->ym; j++) {
    for(i = info->xs; i < info->xs+info->xm; i++) {
      PetscScalar values[3];

      func((PetscReal *) &coords[j][i], values);
      f[j][i].u = values[0];
      f[j][i].v = values[1];
      f[j][i].p = values[2];
    }
  }
  ierr = DAVecRestoreArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}

#undef __FUNCT__
#define __FUNCT__ "Function_Structured_3d"
PetscErrorCode Function_Structured_3d(DALocalInfo *info, Field3D **x[], Field3D **f[], void *ctx)
{
  Options       *options = (Options *) ctx;
  void         (*func)(const double *, double *) = options->func;
  DA             coordDA;
  Vec            coordinates;
  DACoor3d    ***coords;
  PetscInt       i, j, k;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DAGetCoordinateDA(info->da, &coordDA);CHKERRQ(ierr);
  ierr = DAGetCoordinates(info->da, &coordinates);CHKERRQ(ierr);
  ierr = DAVecGetArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  for(k = info->zs; k < info->zs+info->zm; k++) {
    for(j = info->ys; j < info->ys+info->ym; j++) {
      for(i = info->xs; i < info->xs+info->xm; i++) {
        PetscScalar values[4];

        func((PetscReal *) &coords[k][j][i], values);
        f[k][j][i].u = values[0];
        f[k][j][i].v = values[1];
        f[k][j][i].w = values[2];
        f[k][j][i].p = values[3];
      }
    }
  }
  ierr = DAVecRestoreArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}

#undef __FUNCT__
#define __FUNCT__ "Function_Unstructured"
PetscErrorCode Function_Unstructured(Mesh mesh, SectionReal section, void *ctx)
{
  Options       *options = (Options *) ctx;
  void         (*func)(const double *, double *) = options->func;
  Obj<ALE::Mesh> m;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  const Obj<ALE::Mesh::real_section_type>& coordinates = m->getRealSection("coordinates");
  const Obj<ALE::Mesh::label_sequence>&    vertices    = m->depthStratum(0);

  for(ALE::Mesh::label_sequence::iterator v_iter = vertices->begin(); v_iter != vertices->end(); ++v_iter) {
    const ALE::Mesh::real_section_type::value_type *coords = coordinates->restrictPoint(*v_iter);
    PetscScalar                                     values[4];

    (*func)(coords, values);
    ierr = SectionRealUpdate(section, *v_iter, values);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "FormFunctions"
PetscErrorCode FormFunctions(DM dm, Options *options)
{
  PetscTruth     flag;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->structured) {
    DA  da = (DA) dm;
    Vec X, F;

    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view_draw", &flag);CHKERRQ(ierr);
    ierr = DAGetGlobalVector(da, &X);CHKERRQ(ierr);
    ierr = DAGetGlobalVector(da, &F);CHKERRQ(ierr);
    if (options->dim == 2) {
      options->func = cos_2d;
      ierr = DAFormFunctionLocal(da, (DALocalFunction1) Function_Structured_2d, X, F, (void *) options);CHKERRQ(ierr);
      if (flag) {ierr = VecView(F, PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);}
    } else {
      options->func = cos_3d;
      ierr = DAFormFunctionLocal(da, (DALocalFunction1) Function_Structured_3d, X, F, (void *) options);CHKERRQ(ierr);
      if (flag) {ierr = VecView(F, PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);}
    }
    ierr = DARestoreGlobalVector(da, &X);CHKERRQ(ierr);
    ierr = DARestoreGlobalVector(da, &F);CHKERRQ(ierr);
  } else {
    Mesh        mesh = (Mesh) dm;
    SectionReal F;
    Obj<ALE::Mesh> m;
    Obj<ALE::Mesh::real_section_type> s;

    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view_vtk", &flag);CHKERRQ(ierr);
    ierr = MeshGetSectionReal(mesh, "default", &F);CHKERRQ(ierr);
    ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
    ierr = SectionRealGetSection(F, s);CHKERRQ(ierr);
    m->setupField(s);
    if (options->dim == 2) {
      options->func = cos_2d;
    } else {
      options->func = cos_3d;
    }
    ierr = Function_Unstructured(mesh, F, (void *) options);CHKERRQ(ierr);
    if (flag) {ierr = ViewSection(mesh, F, "cos.vtk");CHKERRQ(ierr);}
    ierr = SectionRealDestroy(F);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "setupUnstructuredQuadrature"
PetscErrorCode setupUnstructuredQuadrature(const int dim, int *numQuadPoints, const double *quadPoints[], const double *quadWeights[], int *numBasisFuncs, const double *basis[], const double *basisDer[])
{
  PetscFunctionBegin;
  if (dim == 1) {
    if (numQuadPoints) *numQuadPoints = NUM_QUADRATURE_POINTS_0;
    if (quadPoints)    *quadPoints    = points_0;
    if (quadWeights)   *quadWeights   = weights_0;
    if (numBasisFuncs) *numBasisFuncs = NUM_BASIS_FUNCTIONS_0;
    if (basis)         *basis         = Basis_0;
    if (basisDer)      *basisDer      = BasisDerivatives_0;
  } else if (dim == 2) {
    if (numQuadPoints) *numQuadPoints = NUM_QUADRATURE_POINTS_1;
    if (quadPoints)    *quadPoints    = points_1;
    if (quadWeights)   *quadWeights   = weights_1;
    if (numBasisFuncs) *numBasisFuncs = NUM_BASIS_FUNCTIONS_1;
    if (basis)         *basis         = Basis_1;
    if (basisDer)      *basisDer      = BasisDerivatives_1;
  } else if (dim == 3) {
    if (numQuadPoints) *numQuadPoints = NUM_QUADRATURE_POINTS_2;
    if (quadPoints)    *quadPoints    = points_2;
    if (quadWeights)   *quadWeights   = weights_2;
    if (numBasisFuncs) *numBasisFuncs = NUM_BASIS_FUNCTIONS_2;
    if (basis)         *basis         = Basis_2;
    if (basisDer)      *basisDer      = BasisDerivatives_2;
  } else {
    SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", dim);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "Rhs_Structured_2d_FD"
PetscErrorCode Rhs_Structured_2d_FD(DALocalInfo *info, Field2D *x[], Field2D *f[], void *ctx)
{
  Options       *options = (Options *) ctx;
  void         (*func)(const double *, double *)   = options->func;
  void         (*bcFunc)(const double *, double *) = options->exactFunc;
  DA             coordDA;
  Vec            coordinates;
  DACoor2d     **coords;
  PetscScalar    values[3];
  PetscReal      hxa, hxb, hx, hya, hyb, hy;
  PetscInt       ie = info->xs+info->xm;
  PetscInt       je = info->ys+info->ym;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DAGetCoordinateDA(info->da, &coordDA);CHKERRQ(ierr);
  ierr = DAGetGhostedCoordinates(info->da, &coordinates);CHKERRQ(ierr);
  ierr = DAVecGetArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  // Loop over stencils
  for(int j = info->ys; j < je; j++) {
    for(int i = info->xs; i < ie; i++) {
      if (i == 0 || j == 0 || i == info->mx-1 || j == info->my-1) {
        bcFunc((PetscReal *) &coords[j][i], values);
        f[j][i].u = x[j][i].u - values[0];
        f[j][i].v = x[j][i].v - values[1];
        f[j][i].p = x[j][i].p - values[2];
      } else {
        func((PetscReal *) &coords[j][i], values);
        hya = coords[j+1][i].y - coords[j][i].y;
        hyb = coords[j][i].y   - coords[j-1][i].y;
        hxa = coords[j][i+1].x - coords[j][i].x;
        hxb = coords[j][i].x   - coords[j][i-1].x;
        hy  = 0.5*(hya+hyb);
        hx  = 0.5*(hxa+hxb);
        f[j][i].u = -values[0]*hx*hy -
          ((x[j][i+1].u - x[j][i].u)/hxa - (x[j][i].u - x[j][i-1].u)/hxb)*hy -
          ((x[j+1][i].u - x[j][i].u)/hya - (x[j][i].u - x[j-1][i].u)/hyb)*hx -
          0.5*(x[j][i+1].p - x[j][i-1].p)*hy;
        f[j][i].v = -values[1]*hx*hy -
          ((x[j][i+1].v - x[j][i].v)/hxa - (x[j][i].v - x[j][i-1].v)/hxb)*hy -
          ((x[j+1][i].v - x[j][i].v)/hya - (x[j][i].v - x[j-1][i].v)/hyb)*hx -
          0.5*(x[j+1][i].v - x[j-1][i].v)*hx;
        f[j][i].p = -values[2]*hx*hy +
          0.5*(x[j+1][i].v - x[j-1][i].v)*hx +
          0.5*(x[j][i+1].u - x[j][i-1].u)*hy;
      }
    }
  }
  ierr = DAVecRestoreArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}

#undef __FUNCT__
#define __FUNCT__ "Rhs_Structured_3d_FD"
PetscErrorCode Rhs_Structured_3d_FD(DALocalInfo *info, Field3D **x[], Field3D **f[], void *ctx)
{
  Options       *options = (Options *) ctx;
  void         (*func)(const double *, double *)   = options->func;
  void         (*bcFunc)(const double *, double *) = options->exactFunc;
  DA             coordDA;
  Vec            coordinates;
  DACoor3d    ***coords;
  PetscScalar    values[4];
  PetscReal      hxa, hxb, hx, hya, hyb, hy, hza, hzb, hz;
  PetscInt       ie = info->xs+info->xm;
  PetscInt       je = info->ys+info->ym;
  PetscInt       ke = info->zs+info->zm;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DAGetCoordinateDA(info->da, &coordDA);CHKERRQ(ierr);
  ierr = DAGetGhostedCoordinates(info->da, &coordinates);CHKERRQ(ierr);
  ierr = DAVecGetArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  // Loop over stencils
  for(int k = info->zs; k < ke; k++) {
    for(int j = info->ys; j < je; j++) {
      for(int i = info->xs; i < ie; i++) {
        if (i == 0 || j == 0 || k == 0 || i == info->mx-1 || j == info->my-1 || k == info->mz-1) {
          bcFunc((PetscReal *) &coords[k][j][i], values);
          f[k][j][i].u = x[k][j][i].u - values[0];
          f[k][j][i].v = x[k][j][i].v - values[1];
          f[k][j][i].w = x[k][j][i].w - values[2];
          f[k][j][i].p = x[k][j][i].p - values[3];
        } else {
          func((PetscReal *) &coords[k][j][i], values);
          hza = coords[k+1][j][i].z - coords[k][j][i].z;
          hzb = coords[k][j][i].z   - coords[k-1][j][i].z;
          hya = coords[k][j+1][i].y - coords[k][j][i].y;
          hyb = coords[k][j][i].y   - coords[k][j-1][i].y;
          hxa = coords[k][j][i+1].x - coords[k][j][i].x;
          hxb = coords[k][j][i].x   - coords[k][j][i-1].x;
          hz  = 0.5*(hza+hzb);
          hy  = 0.5*(hya+hyb);
          hx  = 0.5*(hxa+hxb);
        f[k][j][i].u = -values[0]*hx*hy*hz -
          ((x[k][j][i+1].u - x[k][j][i].u)/hxa - (x[k][j][i].u - x[k][j][i-1].u)/hxb)*hy*hz -
          ((x[k][j+1][i].u - x[k][j][i].u)/hya - (x[k][j][i].u - x[k][j-1][i].u)/hyb)*hx*hz - 
          ((x[k+1][j][i].u - x[k][j][i].u)/hza - (x[k][j][i].u - x[k-1][j][i].u)/hzb)*hx*hy -
          0.5*(x[k][j][i+1].p - x[k][j][i-1].p)*hy*hz;
        f[k][j][i].v = -values[1]*hx*hy*hz -
          ((x[k][j][i+1].v - x[k][j][i].v)/hxa - (x[k][j][i].v - x[k][j][i-1].v)/hxb)*hy*hz -
          ((x[k][j+1][i].v - x[k][j][i].v)/hya - (x[k][j][i].v - x[k][j-1][i].v)/hyb)*hx*hz - 
          ((x[k+1][j][i].v - x[k][j][i].v)/hza - (x[k][j][i].v - x[k-1][j][i].v)/hzb)*hx*hy -
          0.5*(x[k][j+1][i].p - x[k][j-1][i].p)*hx*hz;
        f[k][j][i].w = -values[2]*hx*hy*hz -
          ((x[k][j][i+1].w - x[k][j][i].w)/hxa - (x[k][j][i].w - x[k][j][i-1].w)/hxb)*hy*hz -
          ((x[k][j+1][i].w - x[k][j][i].w)/hya - (x[k][j][i].w - x[k][j-1][i].w)/hyb)*hx*hz - 
          ((x[k+1][j][i].w - x[k][j][i].w)/hza - (x[k][j][i].w - x[k-1][j][i].w)/hzb)*hx*hy -
          0.5*(x[k+1][j][i].p - x[k-1][j][i].p)*hx*hy;
        f[k][j][i].p = -values[3]*hx*hy*hz +
          0.5*(x[k+1][j][i].w - x[k-1][j][i].w)*hx*hy +
          0.5*(x[k][j+1][i].v - x[k][j-1][i].v)*hx*hz +
          0.5*(x[k][j][i+1].u - x[k][j][i-1].u)*hy*hz;
        }
      }
    }
  }
  ierr = DAVecRestoreArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}

#undef __FUNCT__
#define __FUNCT__ "Rhs_Unstructured"
PetscErrorCode Rhs_Unstructured(Mesh mesh, SectionReal X, SectionReal section, void *ctx)
{
  Options       *options = (Options *) ctx;
  void         (*func)(const double *, double *) = options->func;
  Obj<ALE::Mesh> m;
  int            numQuadPoints, numBasisFuncs;
  const double  *quadPoints, *quadWeights, *basis, *basisDer;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = setupUnstructuredQuadrature(options->dim, &numQuadPoints, &quadPoints, &quadWeights, &numBasisFuncs, &basis, &basisDer);CHKERRQ(ierr);
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  const Obj<ALE::Mesh::real_section_type>& coordinates = m->getRealSection("coordinates");
  const Obj<ALE::Mesh::label_sequence>&    cells       = m->heightStratum(0);
  const int                                dim         = m->getDimension();
  double      *t_der, *b_der, *coords, *v0, *J, *invJ, detJ;
  PetscScalar *elemVec, *elemMat;

  ierr = SectionRealZero(section);CHKERRQ(ierr);
  ierr = PetscMalloc2(numBasisFuncs,PetscScalar,&elemVec,numBasisFuncs*numBasisFuncs,PetscScalar,&elemMat);CHKERRQ(ierr);
  ierr = PetscMalloc6(dim,double,&t_der,dim,double,&b_der,dim,double,&coords,dim,double,&v0,dim*dim,double,&J,dim*dim,double,&invJ);CHKERRQ(ierr);
  // Loop over cells
  for(ALE::Mesh::label_sequence::iterator c_iter = cells->begin(); c_iter != cells->end(); ++c_iter) {
    ierr = PetscMemzero(elemVec, numBasisFuncs * sizeof(PetscScalar));CHKERRQ(ierr);
    ierr = PetscMemzero(elemMat, numBasisFuncs*numBasisFuncs * sizeof(PetscScalar));CHKERRQ(ierr);
    m->computeElementGeometry(coordinates, *c_iter, v0, J, invJ, detJ);
    if (detJ < 0) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE, "Invalid determinant %g for element %d", detJ, *c_iter);
    // Loop over quadrature points
    for(int q = 0; q < numQuadPoints; ++q) {
      for(int d = 0; d < dim; d++) {
        coords[d] = v0[d];
        for(int e = 0; e < dim; e++) {
          coords[d] += J[d*dim+e]*(quadPoints[q*dim+e] + 1.0);
        }
      }
      PetscScalar funcVals[4];

      (*func)(coords, funcVals);
      // Loop over trial functions
      for(int f = 0; f < numBasisFuncs; ++f) {
        // Constant part
        elemVec[f] -= basis[q*numBasisFuncs+f]*funcVals[0]*quadWeights[q]*detJ;
        // Linear part
        for(int d = 0; d < dim; ++d) {
          t_der[d] = 0.0;
          for(int e = 0; e < dim; ++e) t_der[d] += invJ[e*dim+d]*basisDer[(q*numBasisFuncs+f)*dim+e];
        }
        // Loop over basis functions
        for(int g = 0; g < numBasisFuncs; ++g) {
          // Linear part
          for(int d = 0; d < dim; ++d) {
            b_der[d] = 0.0;
            for(int e = 0; e < dim; ++e) b_der[d] += invJ[e*dim+d]*basisDer[(q*numBasisFuncs+g)*dim+e];
          }
          PetscScalar product = 0.0;
          for(int d = 0; d < dim; ++d) product += t_der[d]*b_der[d];
          elemMat[f*numBasisFuncs+g] += product*quadWeights[q]*detJ;
        }
      }
    }    
    PetscScalar *x;

    ierr = SectionRealRestrict(X, *c_iter, &x);CHKERRQ(ierr);
    // Add linear contribution
    for(int f = 0; f < numBasisFuncs; ++f) {
      for(int g = 0; g < numBasisFuncs; ++g) {
        elemVec[f] += elemMat[f*numBasisFuncs+g]*x[g];
      }
    }
    ierr = SectionRealUpdateAdd(section, *c_iter, elemVec);CHKERRQ(ierr);
  }
  ierr = PetscFree2(elemVec,elemMat);CHKERRQ(ierr);
  ierr = PetscFree6(t_der,b_der,coords,v0,J,invJ);CHKERRQ(ierr);
  // Exchange neighbors
  ierr = SectionRealComplete(section);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "CalculateError"
PetscErrorCode CalculateError(Mesh mesh, SectionReal X, double *error, void *ctx)
{
  Options       *options = (Options *) ctx;
  void         (*func)(const double *, double *) = options->exactFunc;
  Obj<ALE::Mesh> m;
  int            numQuadPoints, numBasisFuncs;
  const double  *quadPoints, *quadWeights, *basis, *basisDer;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = setupUnstructuredQuadrature(options->dim, &numQuadPoints, &quadPoints, &quadWeights, &numBasisFuncs, &basis, &basisDer);CHKERRQ(ierr);
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  const Obj<ALE::Mesh::real_section_type>& coordinates = m->getRealSection("coordinates");
  const Obj<ALE::Mesh::label_sequence>&    cells       = m->heightStratum(0);
  const int                                dim         = m->getDimension();
  double *coords, *v0, *J, *invJ, detJ;
  double  localError = 0.0;

  ierr = PetscMalloc4(dim,double,&coords,dim,double,&v0,dim*dim,double,&J,dim*dim,double,&invJ);CHKERRQ(ierr);
  // Loop over cells
  for(ALE::Mesh::label_sequence::iterator c_iter = cells->begin(); c_iter != cells->end(); ++c_iter) {
    PetscScalar *x;
    double       elemError = 0.0;

    m->computeElementGeometry(coordinates, *c_iter, v0, J, invJ, detJ);
    ierr = SectionRealRestrict(X, *c_iter, &x);CHKERRQ(ierr);
    // Loop over quadrature points
    for(int q = 0; q < numQuadPoints; ++q) {
      for(int d = 0; d < dim; d++) {
        coords[d] = v0[d];
        for(int e = 0; e < dim; e++) {
          coords[d] += J[d*dim+e]*(quadPoints[q*dim+e] + 1.0);
        }
      }
      double      interpolant = 0.0;
      PetscScalar funcVals[4];

      (*func)(coords, funcVals);
      for(int f = 0; f < numBasisFuncs; ++f) {
        interpolant += x[f]*basis[q*numBasisFuncs+f];
      }
      elemError += (interpolant - funcVals[0])*(interpolant - funcVals[0])*quadWeights[q];
    }    
    if (options->debug) {
      std::cout << "Element " << *c_iter << " error: " << elemError << std::endl;
    }
    localError += elemError;
  }
  ierr = MPI_Allreduce(&localError, error, 1, MPI_DOUBLE, MPI_SUM, m->comm());CHKERRQ(ierr);
  ierr = PetscFree4(coords,v0,J,invJ);CHKERRQ(ierr);
  *error = sqrt(*error);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "FormWeakForms"
PetscErrorCode FormWeakForms(DM dm, Options *options)
{
  PetscTruth     flag;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->structured) {
    DA  da = (DA) dm;
    Vec X, F;

    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view_draw", &flag);CHKERRQ(ierr);
    ierr = DAGetGlobalVector(da, &X);CHKERRQ(ierr);
    ierr = DAGetGlobalVector(da, &F);CHKERRQ(ierr);
    ierr = VecSet(F, 0.0);CHKERRQ(ierr);
    if (options->dim == 2) {
      options->func      = cos_2d;
      options->exactFunc = cos_2d;
      ierr = DAFormFunctionLocalGhost(da, (DALocalFunction1) Rhs_Structured_2d_FD, X, F, (void *) options);CHKERRQ(ierr);
      if (flag) {ierr = VecView(F, PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);}
    } else if (options->dim == 3) {
      options->func      = cos_3d;
      options->exactFunc = cos_3d;
      ierr = DAFormFunctionLocalGhost(da, (DALocalFunction1) Rhs_Structured_3d_FD, X, F, (void *) options);CHKERRQ(ierr);
      if (flag) {ierr = VecView(F, PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);}
    } else {
      SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", options->dim);
    }
    ierr = DARestoreGlobalVector(da, &X);CHKERRQ(ierr);
    ierr = DARestoreGlobalVector(da, &F);CHKERRQ(ierr);
  } else {
    Mesh        mesh = (Mesh) dm;
    SectionReal X, F;

    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view_vtk", &flag);CHKERRQ(ierr);
    ierr = MeshGetSectionReal(mesh, "default", &X);CHKERRQ(ierr);
    ierr = SectionRealZero(X);CHKERRQ(ierr);
    ierr = SectionRealDuplicate(X, &F);CHKERRQ(ierr);
    if (options->dim == 2) {
      options->func = cos_2d;
    } else if (options->dim == 3) {
      options->func = cos_3d;
    }
    ierr = Rhs_Unstructured(mesh, X, F, (void *) options);CHKERRQ(ierr);
    if (flag) {ierr = ViewSection(mesh, F, "rhs_cos.vtk");CHKERRQ(ierr);}
    ierr = SectionRealDestroy(F);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "Jac_Structured_2d_FD"
PetscErrorCode Jac_Structured_2d_FD(DALocalInfo *info, PetscScalar *x[], Mat J, void *ctx)
{
  //Options       *options = (Options *) ctx;
  DA             coordDA;
  Vec            coordinates;
  DACoor2d     **coords;
  MatStencil     row, col[7];
  PetscScalar    v[7];
  PetscReal      hxa, hxb, hx, hya, hyb, hy;
  PetscInt       ie = info->xs+info->xm;
  PetscInt       je = info->ys+info->ym;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DAGetCoordinateDA(info->da, &coordDA);CHKERRQ(ierr);
  ierr = DAGetGhostedCoordinates(info->da, &coordinates);CHKERRQ(ierr);
  ierr = DAVecGetArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  // Loop over stencils
  for(int j = info->ys; j < je; j++) {
    for(int i = info->xs; i < ie; i++) {
      row.j = j; row.i = i;
      if (i == 0 || j == 0 || i == info->mx-1 || j == info->my-1) {
        v[0] = 1.0;
        row.c = 0;
        ierr = MatSetValuesStencil(J, 1, &row, 1, &row, v, INSERT_VALUES);CHKERRQ(ierr);
        row.c = 1;
        ierr = MatSetValuesStencil(J, 1, &row, 1, &row, v, INSERT_VALUES);CHKERRQ(ierr);
      } else {
        hya = coords[j+1][i].y - coords[j][i].y;
        hyb = coords[j][i].y   - coords[j-1][i].y;
        hxa = coords[j][i+1].x - coords[j][i].x;
        hxb = coords[j][i].x   - coords[j][i-1].x;
        hy  = 0.5*(hya+hyb);
        hx  = 0.5*(hxa+hxb);
        v[0] = -hx/hyb;                             col[0].j = j - 1; col[0].i = i;
        v[1] = -hy/hxb;                             col[1].j = j;     col[1].i = i - 1;
        v[2] = (hy/hxa + hy/hxb + hx/hya + hx/hyb); col[2].j = row.j; col[2].i = row.i;
        v[3] = -hy/hxa;                             col[3].j = j;     col[3].i = i + 1;
        v[4] = -hx/hya;                             col[4].j = j + 1; col[4].i = i;
        v[5] =  0.5*hy;                             col[5].j = j;     col[5].i = i + 1;
        v[6] = -0.5*hy;                             col[6].j = j;     col[6].i = i - 1;
        row.c = 0; col[0].c = 0; col[1].c = 0; col[2].c = 0; col[3].c = 0; col[4].c = 0; col[5].c = 2; col[6].c = 2;
        ierr = MatSetValuesStencil(J, 1, &row, 7, col, v, INSERT_VALUES);CHKERRQ(ierr);
        v[5] =  0.5*hx;                             col[5].j = j + 1; col[5].i = i;
        v[6] = -0.5*hx;                             col[6].j = j - 1; col[6].i = i;
        row.c = 1; col[0].c = 1; col[1].c = 1; col[2].c = 1; col[3].c = 1; col[4].c = 1; col[5].c = 2; col[6].c = 2;
        ierr = MatSetValuesStencil(J, 1, &row, 7, col, v, INSERT_VALUES);CHKERRQ(ierr);
      }
      if (i == 0) {
        hx = coords[j][i+1].x - coords[j][i].x;
      } else if (i == info->mx-1) {
        hx = coords[j][i].x   - coords[j][i-1].x;
      } else {
        hx = 0.5*(coords[j][i+1].x - coords[j][i-1].x);
      }
      if (j == 0) {
        hy = coords[j+1][i].y - coords[j][i].y;
      } else if (j == info->my-1) {
        hy = coords[j][i].y   - coords[j-1][i].y;
      } else {
        hy = 0.5*(coords[j+1][i].y - coords[j-1][i].y);
      }
      if (j == 0) {
        v[0] =  hx;                                col[0].j = j + 1; col[0].i = i;
        v[1] = -hx;                                col[1].j = j;     col[1].i = i;
      } else if (j == info->my-1) {
        v[0] =  hx;                                col[0].j = j;     col[0].i = i;
        v[1] = -hx;                                col[1].j = j - 1; col[1].i = i;
      } else {
        v[0] =  0.5*hx;                            col[0].j = j + 1; col[0].i = i;
        v[1] = -0.5*hx;                            col[1].j = j - 1; col[1].i = i;
      }
      if (i == 0) {
        v[2] =  hy;                                col[2].j = j;     col[2].i = i + 1;
        v[3] = -hy;                                col[3].j = j;     col[3].i = i;
      } else if (i == info->mx-1) {
        v[2] =  hy;                                col[2].j = j;     col[2].i = i;
        v[3] = -hy;                                col[3].j = j;     col[3].i = i - 1;
      } else {
        v[2] =  0.5*hy;                            col[2].j = j;     col[2].i = i + 1;
        v[3] = -0.5*hy;                            col[3].j = j;     col[3].i = i - 1;
      }
      if (i == 0 && j == 0) {
        v[0] = 1.0;
        row.c = 2;
        ierr = MatSetValuesStencil(J, 1, &row, 1, &row, v, INSERT_VALUES);CHKERRQ(ierr);
      } else {
        row.c = 2; col[0].c = 1; col[1].c = 1; col[2].c = 0; col[3].c = 0;
        ierr = MatSetValuesStencil(J, 1, &row, 4, col, v, INSERT_VALUES);CHKERRQ(ierr);
      }
    }
  }
  ierr = DAVecRestoreArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  ierr = MatAssemblyBegin(J, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(J, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}

#undef __FUNCT__
#define __FUNCT__ "Jac_Structured_3d_FD"
PetscErrorCode Jac_Structured_3d_FD(DALocalInfo *info, PetscScalar *x[], Mat J, void *ctx)
{
  //Options       *options = (Options *) ctx;
  DA             coordDA;
  Vec            coordinates;
  DACoor3d    ***coords;
  MatStencil     row, col[7];
  PetscScalar    v[7];
  PetscReal      hxa, hxb, hx, hya, hyb, hy, hza, hzb, hz;
  PetscInt       ie = info->xs+info->xm;
  PetscInt       je = info->ys+info->ym;
  PetscInt       ke = info->zs+info->zm;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DAGetCoordinateDA(info->da, &coordDA);CHKERRQ(ierr);
  ierr = DAGetGhostedCoordinates(info->da, &coordinates);CHKERRQ(ierr);
  ierr = DAVecGetArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  // Loop over stencils
  for(int k = info->zs; k < ke; k++) {
    for(int j = info->ys; j < je; j++) {
      for(int i = info->xs; i < ie; i++) {
        row.k = k; row.j = j; row.i = i;
        if (i == 0 || j == 0 || k == 0 || i == info->mx-1 || j == info->my-1 || k == info->mz-1) {
          v[0] = 1.0;
          ierr = MatSetValuesStencil(J, 1, &row, 1, &row, v, INSERT_VALUES);CHKERRQ(ierr);
        } else {
          hza = coords[k+1][j][i].z - coords[k][j][i].z;
          hzb = coords[k][j][i].z   - coords[k-1][j][i].z;
          hya = coords[k][j+1][i].y - coords[k][j][i].y;
          hyb = coords[k][j][i].y   - coords[k][j-1][i].y;
          hxa = coords[k][j][i+1].x - coords[k][j][i].x;
          hxb = coords[k][j][i].x   - coords[k][j][i-1].x;
          hz  = 0.5*(hza+hzb);
          hy  = 0.5*(hya+hyb);
          hx  = 0.5*(hxa+hxb);
          v[0] = -hx*hy/hzb;                                       col[0].k = k - 1; col[0].j = j;     col[0].i = i;
          v[1] = -hx*hz/hyb;                                       col[1].k = k;     col[1].j = j - 1; col[1].i = i;
          v[2] = -hy*hz/hxb;                                       col[2].k = k;     col[2].j = j;     col[2].i = i - 1;
          v[3] = (hy*hz/hxa + hy*hz/hxb + hx*hz/hya + hx*hz/hyb + hx*hy/hza + hx*hy/hzb); col[3].k = row.k; col[3].j = row.j; col[3].i = row.i;
          v[4] = -hx*hy/hxa;                                       col[4].k = k + 1; col[4].j = j;     col[4].i = i;
          v[5] = -hx*hz/hya;                                       col[5].k = k;     col[5].j = j + 1; col[5].i = i;
          v[6] = -hy*hz/hxa;                                       col[6].k = k;     col[6].j = j;     col[6].i = i + 1;
          ierr = MatSetValuesStencil(J, 1, &row, 7, col, v, INSERT_VALUES);CHKERRQ(ierr);
        }
      }
    }
  }
  ierr = DAVecRestoreArray(coordDA, coordinates, &coords);CHKERRQ(ierr);
  ierr = MatAssemblyBegin(J, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(J, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "Laplacian_2D_MF"
PetscErrorCode Laplacian_2D_MF(Mat A, Vec x, Vec y)
{
  Mesh             mesh;
  Obj<ALE::Mesh>   m;
  SectionReal      X, Y;
  PetscQuadrature *q;
  PetscErrorCode   ierr;

  PetscFunctionBegin;
  ierr = PetscObjectQuery((PetscObject) A, "mesh", (PetscObject *) &mesh);CHKERRQ(ierr);
  ierr = MatShellGetContext(A, (void **) &q);CHKERRQ(ierr);
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);

  ierr = MeshGetSectionReal(mesh, "work1", &X);CHKERRQ(ierr);
  ierr = MeshGetSectionReal(mesh, "work2", &Y);CHKERRQ(ierr);
  ierr = SectionRealToVec(X, mesh, SCATTER_REVERSE, x);CHKERRQ(ierr);

  const Obj<ALE::Mesh::real_section_type>& coordinates = m->getRealSection("coordinates");
  const Obj<ALE::Mesh::label_sequence>&    cells       = m->heightStratum(0);
  const int     numQuadPoints = q->numQuadPoints;
  const int     numBasisFuncs = q->numBasisFuncs;
  const double *quadWeights   = q->quadWeights;
  const double *basisDer      = q->basisDer;
  const int     dim           = m->getDimension();
  double       *t_der, *b_der, *v0, *J, *invJ, detJ;
  PetscScalar  *elemMat, *elemVec;

  ierr = PetscMalloc2(numBasisFuncs,PetscScalar,&elemVec,numBasisFuncs*numBasisFuncs,PetscScalar,&elemMat);CHKERRQ(ierr);
  ierr = PetscMalloc5(dim,double,&t_der,dim,double,&b_der,dim,double,&v0,dim*dim,double,&J,dim*dim,double,&invJ);CHKERRQ(ierr);
  // Loop over cells
  ierr = SectionRealZero(Y);CHKERRQ(ierr);
  for(ALE::Mesh::label_sequence::iterator c_iter = cells->begin(); c_iter != cells->end(); ++c_iter) {
    ierr = PetscMemzero(elemMat, numBasisFuncs*numBasisFuncs * sizeof(PetscScalar));CHKERRQ(ierr);
    m->computeElementGeometry(coordinates, *c_iter, v0, J, invJ, detJ);
    // Loop over quadrature points
    for(int q = 0; q < numQuadPoints; ++q) {
      // Loop over trial functions
      for(int f = 0; f < numBasisFuncs; ++f) {
        for(int d = 0; d < dim; ++d) {
          t_der[d] = 0.0;
          for(int e = 0; e < dim; ++e) t_der[d] += invJ[e*dim+d]*basisDer[(q*numBasisFuncs+f)*dim+e];
        }
        // Loop over basis functions
        for(int g = 0; g < numBasisFuncs; ++g) {
          for(int d = 0; d < dim; ++d) {
            b_der[d] = 0.0;
            for(int e = 0; e < dim; ++e) b_der[d] += invJ[e*dim+d]*basisDer[(q*numBasisFuncs+g)*dim+e];
          }
          PetscScalar product = 0.0;
          for(int d = 0; d < dim; ++d) product += t_der[d]*b_der[d];
          elemMat[f*numBasisFuncs+g] += product*quadWeights[q]*detJ;
        }
      }
    }
    PetscScalar *ev;

    ierr = SectionRealRestrict(X, *c_iter, &ev);CHKERRQ(ierr);
    // Do local matvec
    for(int f = 0; f < numBasisFuncs; ++f) {
      elemVec[f] = 0.0;
      for(int g = 0; g < numBasisFuncs; ++g) {
        elemVec[f] += elemMat[f*numBasisFuncs+g]*ev[g];
      }
    }
    ierr = SectionRealUpdateAdd(Y, *c_iter, elemVec);CHKERRQ(ierr);
  }
  ierr = PetscFree2(elemVec,elemMat);CHKERRQ(ierr);
  ierr = PetscFree5(t_der,b_der,v0,J,invJ);CHKERRQ(ierr);
  ierr = SectionRealComplete(Y);CHKERRQ(ierr);

  ierr = SectionRealToVec(Y, mesh, SCATTER_FORWARD, y);CHKERRQ(ierr);
  ierr = SectionRealDestroy(X);CHKERRQ(ierr);
  ierr = SectionRealDestroy(Y);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNCT__
#define __FUNCT__ "Jac_Unstructured_Calculated"
PetscErrorCode Jac_Unstructured_Calculated(Mesh mesh, SectionReal section, Mat A, void *ctx)
{
  Options         *options = (Options *) ctx;
  Obj<ALE::Mesh>   m;
  PetscQuadrature *q;
  PetscErrorCode   ierr;

  PetscFunctionBegin;
  ierr = MatShellSetOperation(A, MATOP_MULT, (void(*)(void)) Laplacian_2D_MF);CHKERRQ(ierr);
  ierr = PetscMalloc(sizeof(PetscQuadrature), &q);CHKERRQ(ierr);
  ierr = MatShellSetContext(A, (void *) q);CHKERRQ(ierr);
  ierr = setupUnstructuredQuadrature(options->dim, &q->numQuadPoints, &q->quadPoints, &q->quadWeights,
                                     &q->numBasisFuncs, &q->basis, &q->basisDer);CHKERRQ(ierr);

  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  const Obj<ALE::Mesh::real_section_type>& def   = m->getRealSection("default");
  const Obj<ALE::Mesh::real_section_type>& work1 = m->getRealSection("work1");
  const Obj<ALE::Mesh::real_section_type>& work2 = m->getRealSection("work2");
  work1->setAtlas(def->getAtlas());
  work1->allocateStorage();
  work2->setAtlas(def->getAtlas());
  work2->allocateStorage();
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "Laplacian_2D_MF2"
PetscErrorCode Laplacian_2D_MF2(Mat A, Vec x, Vec y)
{
  Mesh           mesh;
  Obj<ALE::Mesh> m;
  Obj<ALE::Mesh::real_section_type> s;
  SectionReal    op, X, Y;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscObjectQuery((PetscObject) A, "mesh", (PetscObject *) &mesh);CHKERRQ(ierr);
  ierr = MatShellGetContext(A, (void **) &op);CHKERRQ(ierr);
  ierr = SectionRealGetSection(op, s);CHKERRQ(ierr);
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);

  ierr = MeshGetSectionReal(mesh, "work1", &X);CHKERRQ(ierr);
  ierr = MeshGetSectionReal(mesh, "work2", &Y);CHKERRQ(ierr);
  ierr = SectionRealToVec(X, mesh, SCATTER_REVERSE, x);CHKERRQ(ierr);

  const Obj<ALE::Mesh::label_sequence>& cells = m->heightStratum(0);
  const int                             dim   = m->getDimension();
  int           numBasisFuncs;
  PetscScalar  *elemVec;

  ierr = setupUnstructuredQuadrature(dim, 0, 0, 0, &numBasisFuncs, 0, 0);CHKERRQ(ierr);
  ierr = PetscMalloc(numBasisFuncs *sizeof(PetscScalar), &elemVec);CHKERRQ(ierr);
  // Loop over cells
  ierr = SectionRealZero(Y);CHKERRQ(ierr);
  for(ALE::Mesh::label_sequence::iterator c_iter = cells->begin(); c_iter != cells->end(); ++c_iter) {
    const ALE::Mesh::real_section_type::value_type *elemMat = s->restrictPoint(*c_iter);
    PetscScalar *ev;

    ierr = SectionRealRestrict(X,  *c_iter, &ev);CHKERRQ(ierr);
    // Do local matvec
    for(int f = 0; f < numBasisFuncs; ++f) {
      elemVec[f] = 0.0;
      for(int g = 0; g < numBasisFuncs; ++g) {
        elemVec[f] += elemMat[f*numBasisFuncs+g]*ev[g];
      }
    }
    ierr = SectionRealUpdateAdd(Y, *c_iter, elemVec);CHKERRQ(ierr);
  }
  ierr = PetscFree(elemVec);CHKERRQ(ierr);
  ierr = SectionRealComplete(Y);CHKERRQ(ierr);

  ierr = SectionRealToVec(Y, mesh, SCATTER_FORWARD, y);CHKERRQ(ierr);
  ierr = SectionRealDestroy(X);CHKERRQ(ierr);
  ierr = SectionRealDestroy(Y);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNCT__
#define __FUNCT__ "Jac_Unstructured_Stored"
PetscErrorCode Jac_Unstructured_Stored(Mesh mesh, SectionReal section, Mat A, void *ctx)
{
  Options       *options = (Options *) ctx;
  SectionReal    op;
  Obj<ALE::Mesh> m;
  int            numQuadPoints, numBasisFuncs;
  const double  *quadPoints, *quadWeights, *basis, *basisDer;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = setupUnstructuredQuadrature(options->dim, &numQuadPoints, &quadPoints, &quadWeights, &numBasisFuncs, &basis, &basisDer);CHKERRQ(ierr);
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = MatShellSetOperation(A, MATOP_MULT, (void(*)(void)) Laplacian_2D_MF2);CHKERRQ(ierr);
  const Obj<ALE::Mesh::real_section_type>& coordinates = m->getRealSection("coordinates");
  const Obj<ALE::Mesh::label_sequence>&    cells       = m->heightStratum(0);
  const int dim = m->getDimension();
  double      *t_der, *b_der, *v0, *J, *invJ, detJ;
  PetscScalar *elemMat;

  ierr = MeshGetCellSectionReal(mesh, numBasisFuncs*numBasisFuncs, &op);CHKERRQ(ierr);
  ierr = MatShellSetContext(A, (void *) op);CHKERRQ(ierr);
  ierr = PetscMalloc(numBasisFuncs*numBasisFuncs * sizeof(PetscScalar), &elemMat);CHKERRQ(ierr);
  ierr = PetscMalloc5(dim,double,&t_der,dim,double,&b_der,dim,double,&v0,dim*dim,double,&J,dim*dim,double,&invJ);CHKERRQ(ierr);
  // Loop over cells
  for(ALE::Mesh::label_sequence::iterator c_iter = cells->begin(); c_iter != cells->end(); ++c_iter) {
    ierr = PetscMemzero(elemMat, numBasisFuncs*numBasisFuncs * sizeof(PetscScalar));CHKERRQ(ierr);
    m->computeElementGeometry(coordinates, *c_iter, v0, J, invJ, detJ);
    // Loop over quadrature points
    for(int q = 0; q < numQuadPoints; ++q) {
      // Loop over trial functions
      for(int f = 0; f < numBasisFuncs; ++f) {
        for(int d = 0; d < dim; ++d) {
          t_der[d] = 0.0;
          for(int e = 0; e < dim; ++e) t_der[d] += invJ[e*dim+d]*basisDer[(q*numBasisFuncs+f)*dim+e];
        }
        // Loop over basis functions
        for(int g = 0; g < numBasisFuncs; ++g) {
          for(int d = 0; d < dim; ++d) {
            b_der[d] = 0.0;
            for(int e = 0; e < dim; ++e) b_der[d] += invJ[e*dim+d]*basisDer[(q*numBasisFuncs+g)*dim+e];
          }
          PetscScalar product = 0.0;
          for(int d = 0; d < dim; ++d) product += t_der[d]*b_der[d];
          elemMat[f*numBasisFuncs+g] += product*quadWeights[q]*detJ;
        }
      }
    }
    ierr = SectionRealUpdate(op, *c_iter, elemMat);CHKERRQ(ierr);
  }
  ierr = PetscFree(elemMat);CHKERRQ(ierr);
  ierr = PetscFree5(t_der,b_der,v0,J,invJ);CHKERRQ(ierr);

  const Obj<ALE::Mesh::real_section_type>& def   = m->getRealSection("default");
  const Obj<ALE::Mesh::real_section_type>& work1 = m->getRealSection("work1");
  const Obj<ALE::Mesh::real_section_type>& work2 = m->getRealSection("work2");
  work1->setAtlas(def->getAtlas());
  work1->allocateStorage();
  work2->setAtlas(def->getAtlas());
  work2->allocateStorage();
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "Jac_Unstructured"
PetscErrorCode Jac_Unstructured(Mesh mesh, SectionReal section, Mat A, void *ctx)
{
  Options       *options = (Options *) ctx;
  Obj<ALE::Mesh::real_section_type> s;
  Obj<ALE::Mesh> m;
  int            numQuadPoints, numBasisFuncs;
  const double  *quadPoints, *quadWeights, *basis, *basisDer;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = setupUnstructuredQuadrature(options->dim, &numQuadPoints, &quadPoints, &quadWeights, &numBasisFuncs, &basis, &basisDer);CHKERRQ(ierr);
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = SectionRealGetSection(section, s);CHKERRQ(ierr);
  const Obj<ALE::Mesh::real_section_type>& coordinates = m->getRealSection("coordinates");
  const Obj<ALE::Mesh::label_sequence>&    cells       = m->heightStratum(0);
  const Obj<ALE::Mesh::order_type>&        order       = m->getFactory()->getGlobalOrder(m, "default", s);
  const int                                dim         = m->getDimension();
  double      *t_der, *b_der, *v0, *J, *invJ, detJ;
  PetscScalar *elemMat;

  ierr = PetscMalloc(numBasisFuncs*numBasisFuncs * sizeof(PetscScalar), &elemMat);CHKERRQ(ierr);
  ierr = PetscMalloc5(dim,double,&t_der,dim,double,&b_der,dim,double,&v0,dim*dim,double,&J,dim*dim,double,&invJ);CHKERRQ(ierr);
  // Loop over cells
  for(ALE::Mesh::label_sequence::iterator c_iter = cells->begin(); c_iter != cells->end(); ++c_iter) {
    ierr = PetscMemzero(elemMat, numBasisFuncs*numBasisFuncs * sizeof(PetscScalar));CHKERRQ(ierr);
    m->computeElementGeometry(coordinates, *c_iter, v0, J, invJ, detJ);
    // Loop over quadrature points
    for(int q = 0; q < numQuadPoints; ++q) {
      // Loop over trial functions
      for(int f = 0; f < numBasisFuncs; ++f) {
        for(int d = 0; d < dim; ++d) {
          t_der[d] = 0.0;
          for(int e = 0; e < dim; ++e) t_der[d] += invJ[e*dim+d]*basisDer[(q*numBasisFuncs+f)*dim+e];
        }
        // Loop over basis functions
        for(int g = 0; g < numBasisFuncs; ++g) {
          for(int d = 0; d < dim; ++d) {
            b_der[d] = 0.0;
            for(int e = 0; e < dim; ++e) b_der[d] += invJ[e*dim+d]*basisDer[(q*numBasisFuncs+g)*dim+e];
          }
          PetscScalar product = 0.0;
          for(int d = 0; d < dim; ++d) product += t_der[d]*b_der[d];
          elemMat[f*numBasisFuncs+g] += product*quadWeights[q]*detJ;
        }
      }
    }
    ierr = updateOperator(A, m, s, order, *c_iter, elemMat, ADD_VALUES);CHKERRQ(ierr);
  }
  ierr = PetscFree(elemMat);CHKERRQ(ierr);
  ierr = PetscFree5(t_der,b_der,v0,J,invJ);CHKERRQ(ierr);
  ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "FormOperator"
PetscErrorCode FormOperator(DM dm, Options *options)
{
  PetscTruth     flag;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->structured) {
    DA  da = (DA) dm;
    Mat J;
    Vec X;

    ierr = DAGetGlobalVector(da, &X);CHKERRQ(ierr);
    ierr = DAGetMatrix(da, MATAIJ, &J);CHKERRQ(ierr);
    if (options->dim == 2) {
      ierr = DAFormJacobianLocal(da, (DALocalFunction1) Jac_Structured_2d_FD, X, J, options);CHKERRQ(ierr);
    } else if (options->dim == 3) {
      ierr = DAFormJacobianLocal(da, (DALocalFunction1) Jac_Structured_3d_FD, X, J, options);CHKERRQ(ierr);
    } else {
      SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", options->dim);
    }
    ierr = DARestoreGlobalVector(da, &X);CHKERRQ(ierr);
    ierr = MatDestroy(J);CHKERRQ(ierr);
  } else {
    Mesh        mesh = (Mesh) dm;
    SectionReal X;
    Mat         J;

    ierr = PetscOptionsHasName(PETSC_NULL, "-mat_view_draw", &flag);CHKERRQ(ierr);
    ierr = MeshGetSectionReal(mesh, "default", &X);CHKERRQ(ierr);
    ierr = SectionRealZero(X);CHKERRQ(ierr);
    ierr = MeshGetMatrix(mesh, MATAIJ, &J);CHKERRQ(ierr);
    ierr = Jac_Unstructured(mesh, X, J, options);CHKERRQ(ierr);
    if (flag) {ierr = MatView(J, PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);}
    ierr = MatDestroy(J);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "RunTests"
PetscErrorCode RunTests(DM dm, Options *options)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->run == RUN_TEST) {
    ierr = FormFunctions(dm, options);CHKERRQ(ierr);
    ierr = FormWeakForms(dm, options);CHKERRQ(ierr);
    ierr = FormOperator(dm, options);CHKERRQ(ierr);
   }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "CreateProblem"
PetscErrorCode CreateProblem(DM dm, Options *options)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->dim == 2) {
    if (options->bcType == DIRICHLET) {
      options->func      = constant;
      options->exactFunc = quadratic_2d;
    } else {
      SETERRQ(PETSC_ERR_SUP, "No support for Neumann conditions");
    }
  } else if (options->dim == 3) {
    if (options->bcType == DIRICHLET) {
      options->func      = constant;
      options->exactFunc = quadratic_3d;
    } else {
      SETERRQ(PETSC_ERR_SUP, "No support for Neumann conditions");
    }
  } else {
    SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", options->dim);
  }
  if (options->structured) {
    // The DA defines most of the problem during creation
  } else {
    if (options->dim == 1) {
      ierr = CreateProblem_gen_1(dm, options);CHKERRQ(ierr);
      options->integrateP = IntegrateDualBasis_gen_0;
      options->integrateV = IntegrateDualBasis_gen_1;
    } else if (options->dim == 2) {
      ierr = CreateProblem_gen_2(dm, options);CHKERRQ(ierr);
      options->integrateP = IntegrateDualBasis_gen_2;
      ierr = CreateProblem_gen_3(dm, options);CHKERRQ(ierr);
      options->integrateV = IntegrateDualBasis_gen_3;
    } else if (options->dim == 3) {
      ierr = CreateProblem_gen_5(dm, options);CHKERRQ(ierr);
      options->integrateP = IntegrateDualBasis_gen_4;
      options->integrateV = IntegrateDualBasis_gen_5;
    } else {
      SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", options->dim);
    }
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "CreateExactSolution"
PetscErrorCode CreateExactSolution(DM dm, Options *options)
{
  const int      dim = options->dim;
  PetscTruth     flag;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (options->structured) {
    DA da = (DA) dm;
    void (*func)(const double *, double *) = options->func;
    Vec X, U;

    ierr = DAGetGlobalVector(da, &X);CHKERRQ(ierr);
    ierr = DACreateGlobalVector(da, &options->exactSol.vec);CHKERRQ(ierr);
    options->func = options->exactFunc;
    U             = options->exactSol.vec;
    if (dim == 2) {
      ierr = DAFormFunctionLocal(da, (DALocalFunction1) Function_Structured_2d, X, U, (void *) options);CHKERRQ(ierr);
    } else if (dim == 3) {
      ierr = DAFormFunctionLocal(da, (DALocalFunction1) Function_Structured_3d, X, U, (void *) options);CHKERRQ(ierr);
    } else {
      SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", dim);
    }
    ierr = DARestoreGlobalVector(da, &X);CHKERRQ(ierr);
    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view", &flag);CHKERRQ(ierr);
    if (flag) {ierr = VecView(U, PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);}
    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view_draw", &flag);CHKERRQ(ierr);
    if (flag) {ierr = VecView(U, PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);}
    options->func = func;
  } else {
    Mesh mesh = (Mesh) dm;

    Obj<ALE::Mesh> m;
    Obj<ALE::Mesh::real_section_type> s;

    ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
    ierr = MeshGetSectionReal(mesh, "exactSolution", &options->exactSol.section);CHKERRQ(ierr);
    ierr = SectionRealGetSection(options->exactSol.section, s);CHKERRQ(ierr);
    m->setupField(s);
    const Obj<ALE::Mesh::label_sequence>&     cells       = m->heightStratum(0);
    const Obj<ALE::Mesh::real_section_type>&  coordinates = m->getRealSection("coordinates");
    const int                                 localDof    = m->sizeWithBC(s, *cells->begin());
    ALE::Mesh::real_section_type::value_type *values      = new ALE::Mesh::real_section_type::value_type[localDof];
    double                                   *v0          = new double[dim];
    double                                   *J           = new double[dim*dim];
    double                                    detJ;

    for(ALE::Mesh::label_sequence::iterator c_iter = cells->begin(); c_iter != cells->end(); ++c_iter) {
      const Obj<ALE::Mesh::coneArray>      closure = ALE::SieveAlg<ALE::Mesh>::closure(m, *c_iter);
      const ALE::Mesh::coneArray::iterator end     = closure->end();
      int                                  v       = 0;

      m->computeElementGeometry(coordinates, *c_iter, v0, J, PETSC_NULL, detJ);
      for(ALE::Mesh::coneArray::iterator cl_iter = closure->begin(); cl_iter != end; ++cl_iter) {
        // Need to get size from Discretization here
        const int pointDim = s->getFiberDimension(*cl_iter);

        if (pointDim) {
          for(int d = 0; d < pointDim; ++d, ++v) {
            //values[v] = (*options->integrateV)(v0, J, v, options->exactFunc);
          }
        }
      }
      m->updateAll(s, *c_iter, values);
    }
    delete [] values;
    delete [] v0;
    delete [] J;
    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view", &flag);CHKERRQ(ierr);
    if (flag) {s->view("Exact Solution");}
    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view_vtk", &flag);CHKERRQ(ierr);
    if (flag) {ierr = ViewSection(mesh, options->exactSol.section, "exact_sol.vtk");CHKERRQ(ierr);}
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "CheckError"
PetscErrorCode CheckError(DM dm, ExactSolType sol, Options *options)
{
  MPI_Comm       comm;
  const char    *name;
  PetscScalar    norm;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscObjectGetComm((PetscObject) dm, &comm);CHKERRQ(ierr);
  if (options->structured) {
    DA  da = (DA) dm;
    Vec error;

    ierr = DAGetGlobalVector(da, &error);CHKERRQ(ierr);
    ierr = VecCopy(sol.vec, error);CHKERRQ(ierr);
    ierr = VecAXPY(error, -1.0, options->exactSol.vec);CHKERRQ(ierr);
    ierr = VecNorm(error, NORM_2, &norm);CHKERRQ(ierr);
    ierr = DARestoreGlobalVector(da, &error);CHKERRQ(ierr);
    ierr = PetscObjectGetName((PetscObject) sol.vec, &name);CHKERRQ(ierr);
  } else {
    Mesh mesh = (Mesh) dm;

    ierr = CalculateError(mesh, sol.section, &norm, options);CHKERRQ(ierr);
    ierr = PetscObjectGetName((PetscObject) sol.section, &name);CHKERRQ(ierr);
  }
  PetscPrintf(comm, "Error for trial solution %s: %g\n", name, norm);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "CheckResidual"
PetscErrorCode CheckResidual(DM dm, ExactSolType sol, Options *options)
{
  MPI_Comm       comm;
  const char    *name;
  PetscScalar    norm;
  PetscTruth     flag;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view", &flag);CHKERRQ(ierr);
  if (options->structured) {
    DA  da = (DA) dm;
    Vec residual;

    ierr = DAGetGlobalVector(da, &residual);CHKERRQ(ierr);
    ierr = PetscObjectSetName((PetscObject) residual, "residual");CHKERRQ(ierr);
    if (options->dim == 2) {
      ierr = DAFormFunctionLocal(da, (DALocalFunction1) Rhs_Structured_2d_FD, sol.vec, residual, (void *) options);CHKERRQ(ierr);
    } else if (options->dim == 3) {
      ierr = DAFormFunctionLocal(da, (DALocalFunction1) Rhs_Structured_3d_FD, sol.vec, residual, (void *) options);CHKERRQ(ierr);
    } else {
      SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", options->dim);
    }
    ierr = VecNorm(residual, NORM_2, &norm);CHKERRQ(ierr);
    if (flag) {ierr = VecView(residual, PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);}
    ierr = DARestoreGlobalVector(da, &residual);CHKERRQ(ierr);
    ierr = PetscObjectGetName((PetscObject) sol.vec, &name);CHKERRQ(ierr);
  } else {
    Mesh        mesh = (Mesh) dm;
    SectionReal residual;

    ierr = SectionRealDuplicate(sol.section, &residual);CHKERRQ(ierr);
    ierr = PetscObjectSetName((PetscObject) residual, "residual");CHKERRQ(ierr);
    ierr = Rhs_Unstructured(mesh, sol.section, residual, options);CHKERRQ(ierr);
    if (flag) {ierr = SectionRealView(residual, PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);}
    ierr = SectionRealNorm(residual, mesh, NORM_2, &norm);CHKERRQ(ierr);
    ierr = SectionRealDestroy(residual);CHKERRQ(ierr);
    ierr = PetscObjectGetName((PetscObject) sol.section, &name);CHKERRQ(ierr);
  }
  PetscPrintf(comm, "Residual for trial solution %s: %g\n", name, norm);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "CreateSolver"
PetscErrorCode CreateSolver(DM dm, DMMG **dmmg, Options *options)
{
  MPI_Comm       comm;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscObjectGetComm((PetscObject) dm, &comm);CHKERRQ(ierr);
  ierr = DMMGCreate(comm, 1, options, dmmg);CHKERRQ(ierr);
  ierr = DMMGSetDM(*dmmg, dm);CHKERRQ(ierr);
  if (options->structured) {
    // Needed if using finite elements
    // ierr = PetscOptionsSetValue("-dmmg_form_function_ghost", PETSC_NULL);CHKERRQ(ierr);
    if (options->dim == 2) {
      ierr = DMMGSetSNESLocal(*dmmg, Rhs_Structured_2d_FD, Jac_Structured_2d_FD, 0, 0);CHKERRQ(ierr);
    } else if (options->dim == 3) {
      ierr = DMMGSetSNESLocal(*dmmg, Rhs_Structured_3d_FD, Jac_Structured_3d_FD, 0, 0);CHKERRQ(ierr);
    } else {
      SETERRQ1(PETSC_ERR_SUP, "Dimension not supported: %d", options->dim);
    }
    for(int l = 0; l < DMMGGetLevels(*dmmg); l++) {
      ierr = DASetUniformCoordinates((DA) (*dmmg)[l]->dm, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);CHKERRQ(ierr);
    }
  } else {
    if (options->operatorAssembly == ASSEMBLY_FULL) {
      ierr = DMMGSetSNESLocal(*dmmg, Rhs_Unstructured, Jac_Unstructured, 0, 0);CHKERRQ(ierr);
    } else if (options->operatorAssembly == ASSEMBLY_CALCULATED) {
      ierr = DMMGSetMatType(*dmmg, MATSHELL);CHKERRQ(ierr);
      ierr = DMMGSetSNESLocal(*dmmg, Rhs_Unstructured, Jac_Unstructured_Calculated, 0, 0);CHKERRQ(ierr);
    } else if (options->operatorAssembly == ASSEMBLY_STORED) {
      ierr = DMMGSetMatType(*dmmg, MATSHELL);CHKERRQ(ierr);
      ierr = DMMGSetSNESLocal(*dmmg, Rhs_Unstructured, Jac_Unstructured_Stored, 0, 0);CHKERRQ(ierr);
    } else {
      SETERRQ1(PETSC_ERR_ARG_WRONG, "Assembly type not supported: %d", options->operatorAssembly);
    }
  }
  if (options->bcType == NEUMANN) {
    // With Neumann conditions, we tell DMMG that constants are in the null space of the operator
    ierr = DMMGSetNullSpace(*dmmg, PETSC_TRUE, 0, PETSC_NULL);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "Solve"
PetscErrorCode Solve(DMMG *dmmg, Options *options)
{
  SNES                snes;
  MPI_Comm            comm;
  PetscInt            its;
  PetscTruth          flag;
  SNESConvergedReason reason;
  PetscErrorCode      ierr;

  PetscFunctionBegin;
  ierr = DMMGSolve(dmmg);CHKERRQ(ierr);
  snes = DMMGGetSNES(dmmg);
  ierr = SNESGetIterationNumber(snes, &its);CHKERRQ(ierr);
  ierr = SNESGetConvergedReason(snes, &reason);CHKERRQ(ierr);
  ierr = PetscObjectGetComm((PetscObject) snes, &comm);CHKERRQ(ierr);
  ierr = PetscPrintf(comm, "Number of Newton iterations = %D\n", its);CHKERRQ(ierr);
  ierr = PetscPrintf(comm, "Reason for solver termination: %s\n", SNESConvergedReasons[reason]);CHKERRQ(ierr);
  ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view", &flag);CHKERRQ(ierr);
  if (flag) {ierr = VecView(DMMGGetx(dmmg), PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);}
  ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view_draw", &flag);CHKERRQ(ierr);
  if (flag && options->dim == 2) {ierr = VecView(DMMGGetx(dmmg), PETSC_VIEWER_DRAW_WORLD);CHKERRQ(ierr);}
  if (options->structured) {
    ExactSolType sol;

    sol.vec = DMMGGetx(dmmg);
    ierr = CheckError(DMMGGetDM(dmmg), sol, options);CHKERRQ(ierr);
  } else {
    Mesh        mesh = (Mesh) DMMGGetDM(dmmg);
    SectionReal solution;
    Obj<ALE::Mesh::real_section_type> sol;
    double      error;

    ierr = MeshGetSectionReal(mesh, "default", &solution);CHKERRQ(ierr);
    ierr = SectionRealGetSection(solution, sol);CHKERRQ(ierr);
    ierr = SectionRealToVec(solution, mesh, SCATTER_REVERSE, DMMGGetx(dmmg));CHKERRQ(ierr);
    ierr = CalculateError(mesh, solution, &error, options);CHKERRQ(ierr);
    ierr = PetscPrintf(sol->comm(), "Total error: %g\n", error);CHKERRQ(ierr);
    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view_vtk", &flag);CHKERRQ(ierr);
    if (flag) {ierr = ViewSection(mesh, solution, "sol.vtk");CHKERRQ(ierr);}
    ierr = PetscOptionsHasName(PETSC_NULL, "-vec_view", &flag);CHKERRQ(ierr);
    if (flag) {sol->view("Solution");}
    ierr = SectionRealDestroy(solution);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc, char *argv[])
{
  MPI_Comm       comm;
  Options        options;
  DM             dm;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscInitialize(&argc, &argv, (char *) 0, help);CHKERRQ(ierr);
  comm = PETSC_COMM_WORLD;
  ierr = ProcessOptions(comm, &options);CHKERRQ(ierr);
  try {
    ierr = CreateMesh(comm, &dm, &options);CHKERRQ(ierr);
    ierr = CreateProblem(dm, &options);CHKERRQ(ierr);
    ierr = RunTests(dm, &options);CHKERRQ(ierr);
    if (options.run == RUN_FULL) {
      DMMG *dmmg;

      ierr = CreateExactSolution(dm, &options);CHKERRQ(ierr);
      ierr = CheckError(dm, options.exactSol, &options);CHKERRQ(ierr);
      ierr = CheckResidual(dm, options.exactSol, &options);CHKERRQ(ierr);
      ierr = CreateSolver(dm, &dmmg, &options);CHKERRQ(ierr);
      ierr = Solve(dmmg, &options);CHKERRQ(ierr);
      ierr = DMMGDestroy(dmmg);CHKERRQ(ierr);
      ierr = DestroyExactSolution(options.exactSol, &options);CHKERRQ(ierr);
    }
    ierr = DestroyMesh(dm, &options);CHKERRQ(ierr);
  } catch(ALE::Exception e) {
    std::cerr << e << std::endl;
  }
  ierr = PetscFinalize();CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
