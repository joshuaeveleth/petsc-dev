#ifndef included_ALE_Completion_hh
#define included_ALE_Completion_hh

#ifndef  included_ALE_CoSieve_hh
#include <CoSieve.hh>
#endif

namespace ALE {
  namespace New {
    class Completion {
    public:
      typedef ALE::Test::OverlapTest::topology_type     mesh_topology_type;
      typedef ALE::Test::OverlapTest::sieve_type        sieve_type;
      typedef ALE::Test::OverlapTest::dsieve_type       dsieve_type;
      typedef ALE::Test::OverlapTest::send_overlap_type send_overlap_type;
      typedef ALE::Test::OverlapTest::send_section_type send_section_type;
      typedef ALE::Test::OverlapTest::recv_overlap_type recv_overlap_type;
      typedef ALE::Test::OverlapTest::recv_section_type recv_section_type;
      typedef send_section_type::topology_type          topology_type;
      typedef ALE::New::ConstantSection<topology_type, send_section_type::value_type> constant_section;
      typedef ALE::Test::PartitionSizeSection<topology_type, short int>               partition_size_section;
      typedef ALE::Test::PartitionSection<topology_type, short int>                   partition_section;
      typedef ALE::Test::ConeSizeSection<topology_type, sieve_type>                   cone_size_section;
      typedef ALE::Test::ConeSection<topology_type, sieve_type>                       cone_section;
    public:
      static Obj<topology_type> createSendTopology(const Obj<send_overlap_type>& sendOverlap) {
        const Obj<send_overlap_type::traits::baseSequence> ranks = sendOverlap->base();
        Obj<topology_type> topology = new topology_type(sendOverlap->comm(), sendOverlap->debug);

        for(send_overlap_type::traits::baseSequence::iterator r_iter = ranks->begin(); r_iter != ranks->end(); ++r_iter) {
          Obj<dsieve_type> sendSieve = new dsieve_type(sendOverlap->cone(*r_iter));
          topology->setPatch(*r_iter, sendSieve);
        }
        topology->stratify();
        return topology;
      };
      template<typename Sizer>
      static void setupSend(const Obj<send_overlap_type>& sendOverlap, const Obj<Sizer>& sendSizer, const Obj<send_section_type>& sendSection) {
        // Here we should just use the overlap as the topology (once it is a new-style sieve)
        sendSection->getAtlas()->clear();
        sendSection->getAtlas()->setTopology(ALE::New::Completion::createSendTopology(sendOverlap));
        if (sendSection->debug() > 10) {sendSection->getAtlas()->getTopology()->view("Send topology after setup", MPI_COMM_SELF);}
        sendSection->construct(sendSizer);
        sendSection->getAtlas()->orderPatches();
        sendSection->allocate();
        sendSection->constructCommunication(send_section_type::SEND);
      };
      template<typename Filler>
      static void completeSend(const Obj<Filler>& sendFiller, const Obj<send_section_type>& sendSection) {
        // Fill section
        const topology_type::sheaf_type& patches = sendSection->getAtlas()->getTopology()->getPatches();

        for(topology_type::sheaf_type::const_iterator p_iter = patches.begin(); p_iter != patches.end(); ++p_iter) {
          const Obj<topology_type::sieve_type::baseSequence>& base = p_iter->second->base();

          for(topology_type::sieve_type::baseSequence::iterator b_iter = base->begin(); b_iter != base->end(); ++b_iter) {
            sendSection->update(p_iter->first, *b_iter, sendFiller->restrict(p_iter->first, *b_iter));
          }
        }
        if (sendSection->debug()) {sendSection->view("Send Section in Completion", MPI_COMM_SELF);}
        // Complete the section
        sendSection->startCommunication();
        sendSection->endCommunication();
      };
      template<typename SizerFiller, typename Filler>
      static void sendSection(const Obj<send_overlap_type>& sendOverlap, const Obj<SizerFiller>& sizerFiller, const Obj<Filler>& filler, const Obj<send_section_type>& sendSection) {
        Obj<send_section_type> sendSizer     = new send_section_type(sendSection->comm(), sendSection->debug());
        Obj<constant_section>  constantSizer = new constant_section(MPI_COMM_SELF, 1, sendSection->debug());

        // 1) Create the sizer section
        ALE::New::Completion::setupSend(sendOverlap, constantSizer, sendSizer);
        // 2) Fill the sizer section and communicate
        ALE::New::Completion::completeSend(sizerFiller, sendSizer);
        // 3) Create the send section
        ALE::New::Completion::setupSend(sendOverlap, sendSizer, sendSection);
        // 4) Fill up send section and communicate
        ALE::New::Completion::completeSend(filler, sendSection);
      };
      static void sendDistribution(const Obj<mesh_topology_type>& topology, const int dim, const Obj<mesh_topology_type>& topologyNew) {
        const Obj<sieve_type>& sieve         = topology->getPatch(0);
        const Obj<sieve_type>& sieveNew      = topologyNew->getPatch(0);
        Obj<send_overlap_type> sendOverlap   = new send_overlap_type(topology->comm(), topology->debug());
        Obj<send_section_type> sendSizer     = new send_section_type(topology->comm(), topology->debug());
        Obj<send_section_type> sendSection   = new send_section_type(topology->comm(), topology->debug());
        Obj<constant_section>  constantSizer = new constant_section(MPI_COMM_SELF, 1, topology->debug());
        int numElements = topology->heightStratum(0, 0)->size();
        int rank        = topology->commRank();
        int debug       = topology->debug();

        // 1) Form partition point overlap a priori
        //      There are arrows to each rank whose color is the partition point (also the rank)
        for(int p = 1; p < sieve->commSize(); p++) {
          sendOverlap->addCone(p, p, p);
        }
        if (debug) {sendOverlap->view(std::cout, "Send overlap for partition");}
        // 2) Partition the mesh
        short *assignment = ALE::Test::MeshProcessor::partitionSieve_Chaco(topology, dim);
        // 3) Create local sieve
        for(int e = 0; e < numElements; e++) {
          if (assignment[e] == rank) {
            const Obj<sieve_type::traits::coneSequence>& cone = sieve->cone(e);

            for(sieve_type::traits::coneSequence::iterator c_iter = cone->begin(); c_iter != cone->end(); ++c_iter) {
              sieveNew->addArrow(*c_iter, e, c_iter.color());
            }
          }
        }
        // 2) Send the sizer section
        Obj<topology_type>          secTopology          = ALE::New::Completion::createSendTopology(sendOverlap);
        Obj<partition_size_section> partitionSizeSection = new partition_size_section(secTopology, numElements, assignment);
        Obj<partition_section>      partitionSection     = new partition_section(secTopology, numElements, assignment);
        ALE::New::Completion::sendSection(sendOverlap, partitionSizeSection, partitionSection, sendSection);
        // 3) Create point overlap
        // Could this potentially be the sendSection itself?
        sendOverlap->clear();
        const topology_type::sheaf_type& patches = sendSection->getAtlas()->getTopology()->getPatches();

        for(topology_type::sheaf_type::const_iterator p_iter = patches.begin(); p_iter != patches.end(); ++p_iter) {
          const Obj<topology_type::sieve_type::baseSequence>& base = p_iter->second->base();

          for(topology_type::sieve_type::baseSequence::iterator b_iter = base->begin(); b_iter != base->end(); ++b_iter) {
            const send_section_type::value_type *points = sendSection->restrict(p_iter->first, *b_iter);
            int size = sendSection->getAtlas()->size(p_iter->first, *b_iter);

            for(int p = 0; p < size; p++) {
              sendOverlap->addArrow(points[p], p_iter->first, points[p]);
            }
          }
        }
        if (debug) {sendOverlap->view(std::cout, "Send overlap for points");}
        // 4) Send the point section
        secTopology = ALE::New::Completion::createSendTopology(sendOverlap);
        Obj<cone_size_section> coneSizeSection = new cone_size_section(secTopology, sieve);
        Obj<cone_section>      coneSection     = new cone_section(secTopology, sieve);
        ALE::New::Completion::sendSection(sendOverlap, coneSizeSection, coneSection, sendSection);
      };
      template<typename Sizer>
      static void setupReceive(const Obj<recv_overlap_type>& recvOverlap, const Obj<Sizer>& recvSizer, const Obj<recv_section_type>& recvSection) {
        // Create section
        const Obj<recv_overlap_type::traits::capSequence> ranks = recvOverlap->cap();

        recvSection->getAtlas()->clear();
        for(recv_overlap_type::traits::capSequence::iterator r_iter = ranks->begin(); r_iter != ranks->end(); ++r_iter) {
          Obj<dsieve_type> recvSieve = new dsieve_type();
          const Obj<recv_overlap_type::supportSequence>& points = recvOverlap->support(0);

          // Want to replace this loop with a slice through color
          for(recv_overlap_type::supportSequence::iterator p_iter = points->begin(); p_iter != points->end(); ++p_iter) {
            recvSieve->addPoint(p_iter.color());
          }
          recvSection->getAtlas()->getTopology()->setPatch(0, recvSieve);
        }
        recvSection->getAtlas()->getTopology()->stratify();
        recvSection->construct(recvSizer);
        recvSection->getAtlas()->orderPatches();
        recvSection->allocate();
        recvSection->constructCommunication(recv_section_type::RECEIVE);
      };
      static void completeReceive(const Obj<recv_section_type>& recvSection) {
        // Complete the section
        recvSection->startCommunication();
        recvSection->endCommunication();
        if (recvSection->debug()) {recvSection->view("Receive Section in Completion", MPI_COMM_SELF);}
        // Read out section values
      };
      static void recvSection(const Obj<recv_overlap_type>& recvOverlap, const Obj<recv_section_type>& recvSection) {
        Obj<recv_section_type> recvSizer     = new recv_section_type(recvSection->comm(), recvSection->debug());
        Obj<constant_section>  constantSizer = new constant_section(MPI_COMM_SELF, 1, recvSection->debug());

        // 1) Create the sizer section
        ALE::New::Completion::setupReceive(recvOverlap, constantSizer, recvSizer);
        // 2) Communicate
        ALE::New::Completion::completeReceive(recvSizer);
        // 3) Update to the receive section
        ALE::New::Completion::setupReceive(recvOverlap, recvSizer, recvSection);
        // 4) Communicate
        ALE::New::Completion::completeReceive(recvSection);
      };
      static void receiveDistribution(const Obj<mesh_topology_type>& topology, const Obj<mesh_topology_type>& topologyNew) {
        const Obj<sieve_type>& sieve         = topology->getPatch(0);
        const Obj<sieve_type>& sieveNew      = topologyNew->getPatch(0);
        Obj<recv_overlap_type> recvOverlap = new recv_overlap_type(topology->comm(), topology->debug());
        Obj<recv_section_type> recvSizer   = new recv_section_type(topology->comm(), topology->debug());
        Obj<recv_section_type> recvSection = new recv_section_type(topology->comm(), topology->debug());
        Obj<constant_section>  constantSizer = new constant_section(MPI_COMM_SELF, 1, topology->debug());
        int debug = topology->debug();

        // 1) Form partition point overlap a priori
        //      The arrow is from rank 0 with partition point 0
        recvOverlap->addCone(0, sieve->commRank(), sieve->commRank());
        if (debug) {recvOverlap->view(std::cout, "Receive overlap for partition");}
        // 2) Receive sizer section
        ALE::New::Completion::recvSection(recvOverlap, recvSection);
        // 3) Unpack the section into the overlap
        recvOverlap->clear();
        const topology_type::sheaf_type& patches = recvSection->getAtlas()->getTopology()->getPatches();

        for(topology_type::sheaf_type::const_iterator p_iter = patches.begin(); p_iter != patches.end(); ++p_iter) {
          const Obj<topology_type::sieve_type::baseSequence>& base = p_iter->second->base();
          int                                                 rank = p_iter->first;

          for(topology_type::sieve_type::baseSequence::iterator b_iter = base->begin(); b_iter != base->end(); ++b_iter) {
            const recv_section_type::value_type *points = recvSection->restrict(rank, *b_iter);
            int size = recvSection->getAtlas()->getFiberDimension(rank, *b_iter);

            for(int p = 0; p < size; p++) {
              recvOverlap->addArrow(rank, points[p], points[p]);
            }
          }
        }
        if (debug) {recvOverlap->view(std::cout, "Receive overlap for points");}
        // 4) Receive the point section
        ALE::New::Completion::recvSection(recvOverlap, recvSection);
        // 5) Unpack the section into the sieve
        for(topology_type::sheaf_type::const_iterator p_iter = patches.begin(); p_iter != patches.end(); ++p_iter) {
          const Obj<topology_type::sieve_type::baseSequence>& base = p_iter->second->base();
          int                                                     rank = p_iter->first;

          for(topology_type::sieve_type::baseSequence::iterator b_iter = base->begin(); b_iter != base->end(); ++b_iter) {
            const recv_section_type::value_type *points = recvSection->restrict(rank, *b_iter);
            int size = recvSection->getAtlas()->getFiberDimension(rank, *b_iter);
            int c = 0;

            for(int p = 0; p < size; p++) {
              sieveNew->addArrow(points[p], *b_iter, c++);
            }
          }
        }
      };
    };
  }
}
#endif
