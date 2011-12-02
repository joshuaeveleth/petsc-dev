#define PETSCDM_DLL
#include <private/meshimpl.h>    /*I   "petscdmmesh.h"   I*/
#include <petscdmda.h>

#undef __FUNCT__
#define __FUNCT__ "DMSetFromOptions_Mesh"
PetscErrorCode  DMSetFromOptions_Mesh(DM dm)
{
  PetscBool      flg;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscOptionsHead("DMMesh Options");CHKERRQ(ierr);
    /* Handle DMMesh refinement */
    /* Handle associated vectors */
    /* Handle viewing */
    ierr = PetscOptionsBool("-mesh_view_vtk", "Output mesh in VTK format", "DMView", PETSC_FALSE, &flg, PETSC_NULL);CHKERRQ(ierr);
    if (flg) {
      PetscViewer viewer;

      ierr = PetscViewerCreate(((PetscObject) dm)->comm, &viewer);CHKERRQ(ierr);
      ierr = PetscViewerSetType(viewer, PETSCVIEWERASCII);CHKERRQ(ierr);
      ierr = PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_VTK);CHKERRQ(ierr);
      ierr = PetscViewerFileSetName(viewer, "mesh.vtk");CHKERRQ(ierr);
      ierr = DMView(dm, viewer);CHKERRQ(ierr);
      ierr = PetscViewerDestroy(&viewer);CHKERRQ(ierr);
    }
    ierr = PetscOptionsBool("-mesh_view", "Exhaustive mesh description", "DMView", PETSC_FALSE, &flg, PETSC_NULL);CHKERRQ(ierr);
    if (flg) {
      ALE::Obj<PETSC_MESH_TYPE> mesh;

      ierr = DMMeshGetMesh(dm, mesh);CHKERRQ(ierr);
      mesh->view("Mesh");
    }
    ierr = PetscOptionsBool("-mesh_view_simple", "Simple mesh description", "DMView", PETSC_FALSE, &flg, PETSC_NULL);CHKERRQ(ierr);
    if (flg) {
      ierr = DMView(dm, PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);
    }
  ierr = PetscOptionsTail();CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#include <sieve/DMBuilder.hh>

#undef __FUNCT__
#define __FUNCT__ "DMMeshCreateBoxMesh"
PetscErrorCode DMMeshCreateBoxMesh(MPI_Comm comm, PetscInt dim, PetscBool interpolate, DM *dm) {
  PetscInt       debug = 0;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  try {
    ierr = ALE::DMBuilder::createBoxMesh(comm, dim, false, interpolate, debug, dm);CHKERRQ(ierr);
  } catch(ALE::Exception e) {
    SETERRQ1(comm, PETSC_ERR_PLIB, "Unable to create mesh: %s", e.message());
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMeshCreateMeshFromAdjacency"
/*@
  DMMeshCreateMeshFromAdjacency - Create an unstrctured mesh from a list of the vertices for each cell, and the coordinates for each vertex.

 Collective on comm

  Input Parameters:
+ comm - An MPI communicator
. dim - The dimension of the cells, e.g. triangles have dimension 2
. numCells - The number of cells in the mesh
. numCorners - The number of vertices in each cell
. cellVertices - An array of the vertices for each cell, numbered 0 to numVertices-1
. spatialDim - The dimension for coordinates, e.g. for a triangle in 3D this would be 3
. numVertices - The number of mesh vertices
. coordinates - An array of the coordinates for each vertex
- interpolate - Flag to create faces and edges

  Output Parameter:
. dm - The DMMesh object

  Level: beginner

.seealso DMMESH, DMMeshCreateMeshFromAdjacencyHybrid(), DMMeshCreateBoxMesh()
@*/
PetscErrorCode DMMeshCreateMeshFromAdjacency(MPI_Comm comm, PetscInt dim, PetscInt numCells, PetscInt numCorners, PetscInt cellVertices[], PetscInt spatialDim, PetscInt numVertices, const PetscReal coordinates[], PetscBool interpolate, DM *dm) {
  PetscInt       debug = 0;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidPointer(cellVertices, 5);
  /*PetscValidLogicalCollectiveBool(comm,interpolate,6);*/
  PetscValidPointer(dm, 7);
  if (interpolate) {SETERRQ(comm, PETSC_ERR_SUP, "Interpolation (creation of faces and edges) is not yet supported.");}
  ierr = PetscOptionsGetInt(PETSC_NULL, "-dmmesh_debug", &debug, PETSC_NULL);CHKERRQ(ierr);
  Obj<PETSC_MESH_TYPE>             mesh  = new PETSC_MESH_TYPE(comm, dim, debug);
  Obj<PETSC_MESH_TYPE::sieve_type> sieve = new PETSC_MESH_TYPE::sieve_type(comm, 0, numCells+numVertices, debug);

  mesh->setSieve(sieve);
  for(PetscInt c = 0; c < numCells; ++c) {
    sieve->setConeSize(c, numCorners);
  }
  sieve->symmetrizeSizes(numCells, numCorners, cellVertices);
  sieve->allocate();
  for(PetscInt c = 0; c < numCells; ++c) {
    // Need to add numCells
    sieve->setCone(&cellVertices[c*numCorners], c);
  }
  sieve->symmetrize();
  mesh->stratify();
  /* Handle orientation */
  ALE::SieveBuilder<PETSC_MESH_TYPE>::buildCoordinates(mesh, spatialDim, coordinates, numCells);
  ierr = DMCreate(comm, dm);CHKERRQ(ierr);
  ierr = DMSetType(*dm, DMMESH);CHKERRQ(ierr);
  ierr = DMMeshSetMesh(*dm, mesh);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMMeshCreateMeshFromAdjacencyHybrid"
PetscErrorCode DMMeshCreateMeshFromAdjacencyHybrid(MPI_Comm comm, PetscInt dim, PetscInt numCells, PetscInt numCorners[], PetscInt cellVertices[], PetscInt spatialDim, PetscInt numVertices, const PetscReal coordinates[], PetscBool interpolate, DM *dm) {
  PetscInt       debug = 0;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidPointer(numCorners, 4);
  PetscValidPointer(cellVertices, 5);
  /*PetscValidLogicalCollectiveBool(comm,interpolate,6);*/
  PetscValidPointer(dm, 7);
  if (interpolate) {SETERRQ(comm, PETSC_ERR_SUP, "Interpolation (creation of faces and edges) is not yet supported.");}
  ierr = PetscOptionsGetInt(PETSC_NULL, "-dmmesh_debug", &debug, PETSC_NULL);CHKERRQ(ierr);
  Obj<PETSC_MESH_TYPE>             mesh  = new PETSC_MESH_TYPE(comm, dim, debug);
  Obj<PETSC_MESH_TYPE::sieve_type> sieve = new PETSC_MESH_TYPE::sieve_type(comm, 0, numCells+numVertices, debug);

  mesh->setSieve(sieve);
  for(PetscInt c = 0; c < numCells; ++c) {
    sieve->setConeSize(c, numCorners[c]);
  }
  //sieve->symmetrizeSizes(numCells, numCorners, cellVertices);
  sieve->allocate();
  for(PetscInt c = 0, offset = 0; c < numCells; offset += numCorners[c], ++c) {
    sieve->setCone(&cellVertices[offset], c);
  }
  sieve->symmetrize();
  PetscFunctionReturn(0);
}

/* External function declarations here */
extern PetscErrorCode DMGlobalToLocalBegin_Mesh(DM dm, Vec g, InsertMode mode, Vec l);
extern PetscErrorCode DMGlobalToLocalEnd_Mesh(DM dm, Vec g, InsertMode mode, Vec l);
extern PetscErrorCode DMLocalToGlobalBegin_Mesh(DM dm, Vec l, InsertMode mode, Vec g);
extern PetscErrorCode DMLocalToGlobalEnd_Mesh(DM dm, Vec l, InsertMode mode, Vec g);
extern PetscErrorCode DMCreateGlobalVector_Mesh(DM dm, Vec *gvec);
extern PetscErrorCode DMCreateLocalVector_Mesh(DM dm, Vec *lvec);
extern PetscErrorCode DMCreateLocalToGlobalMapping_Mesh(DM dm);
extern PetscErrorCode DMCreateInterpolation_Mesh(DM dmCoarse, DM dmFine, Mat *interpolation, Vec *scaling);
extern PetscErrorCode DMCreateMatrix_Mesh(DM dm, const MatType mtype, Mat *J);
extern PetscErrorCode DMRefine_Mesh(DM dm, MPI_Comm comm, DM *dmRefined);
extern PetscErrorCode DMCoarsenHierarchy_Mesh(DM dm, int numLevels, DM *coarseHierarchy);
extern PetscErrorCode DMDestroy_Mesh(DM dm);
extern PetscErrorCode DMView_Mesh(DM dm, PetscViewer viewer);

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "DMConvert_DA_Mesh"
PetscErrorCode DMConvert_DA_Mesh(DM dm, const DMType newtype, DM *dmNew)
{
  typedef ALE::Mesh<PetscInt,PetscScalar> FlexMesh;
  DM             cda;
  DMDALocalInfo  info;
  Vec            coordinates;
  PetscScalar   *coords;
  PetscInt       M;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = DMDAGetInfo(dm, 0, &M, 0,0,0,0,0,0,0,0,0,0,0);CHKERRQ(ierr);
  ierr = DMDAGetLocalInfo(dm, &info);CHKERRQ(ierr);
  if (info.dim > 1) SETERRQ(((PetscObject) dm)->comm, PETSC_ERR_SUP, "Currently, only 1D DMDAs can be converted to DMMeshes.");
  if (info.sw  > 1) SETERRQ(((PetscObject) dm)->comm, PETSC_ERR_SUP, "Currently, only DMDAs with unti stencil width can be converted to DMMeshes.");
  ierr = DMDAGetCoordinateDA(dm, &cda);CHKERRQ(ierr);
  ierr = DMDAGetGhostedCoordinates(dm, &coordinates);CHKERRQ(ierr);
  ierr = DMDAVecGetArray(dm, coordinates, &coords);CHKERRQ(ierr);
  //ierr = VecGetArray(coordinates, &coords);CHKERRQ(ierr);

  ierr = DMMeshCreate(((PetscObject) dm)->comm, dmNew);CHKERRQ(ierr);
  ALE::Obj<PETSC_MESH_TYPE>              mesh  = new PETSC_MESH_TYPE(((PetscObject) dm)->comm, info.dim, 0);
  ALE::Obj<PETSC_MESH_TYPE::sieve_type>  sieve = new PETSC_MESH_TYPE::sieve_type(((PetscObject) dm)->comm, 0);
  ALE::Obj<FlexMesh>                     m     = new FlexMesh(((PetscObject) dm)->comm, info.dim, 0);
  ALE::Obj<FlexMesh::sieve_type>         s     = new FlexMesh::sieve_type(((PetscObject) dm)->comm, 0);
  PETSC_MESH_TYPE::renumbering_type      renumbering;

  m->setSieve(s);
  {
    /* WE MUST PUT IN HALO VERTICES, BUT MARK THEM SO WE CAN AVOID THEM WHEN ITERATING */
    /* M edges if its periodic */
    /* Edges are numbered     0..M-2
       Vertices are numbered: M-1..2M-2

       For vertex names, we just add M-1
       Edge i connects vertex (i+M-1) to vertex (i+M), do we add info.xs
    */
    const PetscInt        numGlobalEdges = M-1;
    const PetscInt        numVertices    = info.gxm;
    /* const PetscInt        numEdges       = numVertices-1; Do not include edge attached to ghost vertex */
    FlexMesh::point_type *vertices       = new FlexMesh::point_type[numVertices];
    const ALE::Obj<FlexMesh::label_type>& markers = m->createLabel("marker");

    /* Create vertices */
    for(int v = info.gxs; v < info.gxs+info.gxm; ++v) {
      vertices[v-info.gxs] = FlexMesh::point_type(v+numGlobalEdges);
    }
    /* Create edges */
    int order = 0;

    for(int e = 0; e < numVertices-1; ++e) {
      FlexMesh::point_type edge(info.gxs+e);

      s->addArrow(vertices[e],                 edge, order++);
      s->addArrow(vertices[(e+1)%numVertices], edge, order++); /* This must be corrected for periodicity */
    }
    for(int e = 0; e < numVertices-1; ++e) {
      FlexMesh::point_type edge(info.xs+e);

      s->addArrow(vertices[e],                 edge, order++);
      s->addArrow(vertices[(e+1)%numVertices], edge, order++);
    }
    if (info.gxs+info.gxm > info.xs+info.xm) {
      FlexMesh::point_type edge(info.xs+info.xm-1);

      s->addArrow(vertices[numVertices-1],     edge, order++);
    }
    m->stratify();
    /* Mark domain endpoints -- only do this if its not periodic */
    if (vertices[0] == M-1) {
      m->setValue(markers, vertices[0], 1);
    }
    if (vertices[numVertices-1] == 2*M-2) {
      m->setValue(markers, vertices[numVertices-1], 2);
    }
    /* Mark ghost nodes */
    for(int v = info.gxs; v < info.xs; ++v) {
      m->setValue(markers, vertices[v-info.gxs], 3);
    }
    for(int v = info.xs+info.xm; v < info.gxs+info.gxm; ++v) {
      m->setValue(markers, vertices[v-info.gxs], 4);
    }
    delete [] vertices;
    // vertexNumber - (numGlobalEdges - numVertices)
    ALE::SieveBuilder<FlexMesh>::buildCoordinates(m, info.dim, coords, numGlobalEdges);
  }
  mesh->setSieve(sieve);
  m->view("Flexible Mesh");
  ALE::ISieveConverter::convertMesh(*m, *mesh, renumbering, true);
  {
    typedef PETSC_MESH_TYPE::point_type point_type;
    PETSc::Log::Event("CreateOverlap").begin();
    ALE::Obj<PETSC_MESH_TYPE::send_overlap_type> sendParallelMeshOverlap = mesh->getSendOverlap();
    ALE::Obj<PETSC_MESH_TYPE::recv_overlap_type> recvParallelMeshOverlap = mesh->getRecvOverlap();
    //   Can I figure this out in a nicer way?
    ALE::SetFromMap<std::map<point_type,point_type> > globalPoints(renumbering);

    ALE::OverlapBuilder<>::constructOverlap(globalPoints, renumbering, sendParallelMeshOverlap, recvParallelMeshOverlap);
    sendParallelMeshOverlap->view("Send Overlap");
    recvParallelMeshOverlap->view("Recieve Overlap");
    mesh->setCalculatedOverlap(true);
    PETSc::Log::Event("CreateOverlap").end();
  }
  ierr = VecRestoreArray(coordinates, &coords);CHKERRQ(ierr);
  ierr = DMMeshSetMesh(*dmNew, mesh);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "DMCreate_Mesh"
PetscErrorCode DMCreate_Mesh(DM dm)
{
  DM_Mesh       *mesh;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(dm, DM_CLASSID, 1);
  ierr = PetscNewLog(dm, DM_Mesh, &mesh);CHKERRQ(ierr);
  dm->data = mesh;

  new(&mesh->m) ALE::Obj<PETSC_MESH_TYPE>(PETSC_NULL);

  mesh->globalScatter  = PETSC_NULL;
  mesh->defaultSection = PETSC_NULL;
  mesh->lf             = PETSC_NULL;
  mesh->lj             = PETSC_NULL;

  ierr = PetscStrallocpy(VECSTANDARD, &dm->vectype);CHKERRQ(ierr);
  dm->ops->view               = DMView_Mesh;
  dm->ops->setfromoptions     = DMSetFromOptions_Mesh;
  dm->ops->setup              = 0;
  dm->ops->createglobalvector = DMCreateGlobalVector_Mesh;
  dm->ops->createlocalvector  = DMCreateLocalVector_Mesh;
  dm->ops->createlocaltoglobalmapping      = DMCreateLocalToGlobalMapping_Mesh;
  dm->ops->createlocaltoglobalmappingblock = 0;

  dm->ops->getcoloring        = 0;
  dm->ops->getmatrix          = DMCreateMatrix_Mesh;
  dm->ops->getinterpolation   = DMCreateInterpolation_Mesh;
  dm->ops->getaggregates      = 0;
  dm->ops->getinjection       = 0;

  dm->ops->refine             = DMRefine_Mesh;
  dm->ops->coarsen            = 0;
  dm->ops->refinehierarchy    = 0;
  dm->ops->coarsenhierarchy   = DMCoarsenHierarchy_Mesh;

  dm->ops->forminitialguess   = 0;
  dm->ops->formfunction       = 0;

  dm->ops->globaltolocalbegin = DMGlobalToLocalBegin_Mesh;
  dm->ops->globaltolocalend   = DMGlobalToLocalEnd_Mesh;
  dm->ops->localtoglobalbegin = DMLocalToGlobalBegin_Mesh;
  dm->ops->localtoglobalend   = DMLocalToGlobalEnd_Mesh;

  dm->ops->initialguess       = 0;
  dm->ops->function           = 0;
  dm->ops->functionj          = 0;
  dm->ops->jacobian           = 0;

  dm->ops->destroy            = DMDestroy_Mesh;

  ierr = PetscObjectComposeFunction((PetscObject) dm, "DMConvert_da_mesh_C", "DMConvert_DA_Mesh", (void (*)(void)) DMConvert_DA_Mesh);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNCT__
#define __FUNCT__ "DMMeshCreate"
/*@
  DMMeshCreate - Creates a DMMesh object.

  Collective on MPI_Comm

  Input Parameter:
. comm - The communicator for the DMMesh object

  Output Parameter:
. mesh  - The DMMesh object

  Level: beginner

.keywords: DMMesh, create
@*/
PetscErrorCode  DMMeshCreate(MPI_Comm comm, DM *mesh)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidPointer(mesh,2);
  ierr = DMCreate(comm, mesh);CHKERRQ(ierr);
  ierr = DMSetType(*mesh, DMMESH);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
