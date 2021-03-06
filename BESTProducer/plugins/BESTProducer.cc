// -*- C++ -*-
//
// Package:    Analysis/BESTProducer
// Class:      BESTProducer
// 
/**\class BESTProducer BESTProducer.cc Analysis/BESTProducer/plugins/BESTProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  justin pilot
//         Created:  Thu, 07 Jul 2016 11:41:53 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Common/interface/TriggerNames.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "PhysicsTools/CandUtils/interface/EventShapeVariables.h"
#include "PhysicsTools/CandUtils/interface/Thrust.h"

#include <fastjet/JetDefinition.hh>
#include <fastjet/PseudoJet.hh>
#include "fastjet/tools/Filter.hh"
#include <fastjet/ClusterSequence.hh>
#include <fastjet/ActiveAreaSpec.hh>
#include <fastjet/ClusterSequenceArea.hh>

#include "TH2F.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TFile.h"
#include "TCanvas.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"


//
// class declaration
//
class BESTProducer : public edm::stream::EDProducer<> {
  public:
    explicit BESTProducer(const edm::ParameterSet&);
    ~BESTProducer();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:

    virtual void produce(edm::Event&, const edm::EventSetup&) override;

    float LegP(float x, int order);
    int FWMoments( std::vector<TLorentzVector> particles, double (&outputs)[5] );
    void pboost( TVector3 pbeam, TVector3 plab, TLorentzVector &pboo );    

    std::map<std::string, float> treeVars;
    std::vector<std::string> listOfVars;

    int NNtargetX_;
    int NNtargetY_;
    int isMC_;
    int m_year;

    float kappa_ = 0.6;        // jet charge pT-weight
    double AK4_radius_ = 0.4;  // AK4 jet radius
    double AK8_radius_ = 0.8;  // AK8 jet radius
   
    edm::EDGetTokenT<std::vector<pat::PackedCandidate> > pfCandsToken_;
    edm::EDGetTokenT<std::vector<pat::Jet> > ak8JetsToken_;
    edm::EDGetTokenT<std::vector<pat::Jet> > ak4JetsToken_;
    edm::EDGetTokenT<std::vector<pat::Jet> > ak8CHSSoftDropSubjetsToken_;
    edm::EDGetTokenT<std::vector<reco::Vertex> > verticesToken_;
};


//
// constructors and destructor
//
BESTProducer::BESTProducer(const edm::ParameterSet& iConfig):
  NNtargetX_  (iConfig.getParameter<int>("NNtargetX")),
  NNtargetY_  (iConfig.getParameter<int>("NNtargetY")),
  isMC_ (iConfig.getParameter<int>("isMC")),
  m_year(iConfig.getParameter<int>("year")){
    produces<std::vector<float > >("FWmoment0");
    produces<std::vector<float > >("FWmoment1");
    produces<std::vector<float > >("FWmoment2");
    produces<std::vector<float > >("FWmoment3");
    produces<std::vector<float > >("FWmoment4");
    produces<std::vector<float > >("sumPztop");
    produces<std::vector<float > >("sumPzW");
    produces<std::vector<float > >("sumPzZ");
    produces<std::vector<float > >("sumPzH");
    produces<std::vector<float > >("sumPzjet");
    produces<std::vector<float > >("sumPtop");
    produces<std::vector<float > >("sumPW");
    produces<std::vector<float > >("sumPZ");
    produces<std::vector<float > >("sumPH");
    produces<std::vector<float > >("sumPjet");
    produces<std::vector<float > >("Njetstop");
    produces<std::vector<float > >("NjetsW");
    produces<std::vector<float > >("NjetsZ");
    produces<std::vector<float > >("NjetsH");
    produces<std::vector<float > >("Njetsjet");
    produces<std::vector<float > >("FWmoment1top");
    produces<std::vector<float > >("FWmoment2top");
    produces<std::vector<float > >("FWmoment3top");
    produces<std::vector<float > >("FWmoment4top");
    produces<std::vector<float > >("isotropytop");
    produces<std::vector<float > >("sphericitytop");
    produces<std::vector<float > >("aplanaritytop");
    produces<std::vector<float > >("thrusttop");
    produces<std::vector<float > >("FWmoment1W");
    produces<std::vector<float > >("FWmoment2W");
    produces<std::vector<float > >("FWmoment3W");
    produces<std::vector<float > >("FWmoment4W");
    produces<std::vector<float > >("isotropyW");
    produces<std::vector<float > >("sphericityW");
    produces<std::vector<float > >("aplanarityW");
    produces<std::vector<float > >("thrustW");
    produces<std::vector<float > >("FWmoment1Z");
    produces<std::vector<float > >("FWmoment2Z");
    produces<std::vector<float > >("FWmoment3Z");
    produces<std::vector<float > >("FWmoment4Z");
    produces<std::vector<float > >("isotropyZ");
    produces<std::vector<float > >("sphericityZ");
    produces<std::vector<float > >("aplanarityZ");
    produces<std::vector<float > >("thrustZ");
    produces<std::vector<float > >("FWmoment1H");
    produces<std::vector<float > >("FWmoment2H");
    produces<std::vector<float > >("FWmoment3H");
    produces<std::vector<float > >("FWmoment4H");
    produces<std::vector<float > >("isotropyH");
    produces<std::vector<float > >("sphericityH");
    produces<std::vector<float > >("aplanarityH");
    produces<std::vector<float > >("thrustH");
    produces<std::vector<float > >("et");
    produces<std::vector<float > >("eta");
    produces<std::vector<float > >("mass");
    produces<std::vector<float > >("SDmass");
    produces<std::vector<float > >("tau32");
    produces<std::vector<float > >("tau21");
    produces<std::vector<float > >("bDisc");
    produces<std::vector<float > >("bDisc1");
    produces<std::vector<float > >("bDisc2");
    produces<std::vector<float > >("m12H");
    produces<std::vector<float > >("m23H");
    produces<std::vector<float > >("m13H");
    produces<std::vector<float > >("m1234H");
    produces<std::vector<float > >("m12W");
    produces<std::vector<float > >("m23W");
    produces<std::vector<float > >("m13W");
    produces<std::vector<float > >("m1234W");
    produces<std::vector<float > >("m12Z");
    produces<std::vector<float > >("m23Z");
    produces<std::vector<float > >("m13Z");
    produces<std::vector<float > >("m1234Z");
    produces<std::vector<float > >("m12top");
    produces<std::vector<float > >("m23top");
    produces<std::vector<float > >("m13top");
    produces<std::vector<float > >("m1234top");
    produces<std::vector<int > > ("nPV");
    produces<std::vector<int > > ("nAK4Jets");
    produces<std::vector<float > >("q");
    produces<std::vector<float > >("qsubjet0");
    produces<std::vector<float > >("qsubjet1");
    produces<std::vector<int > >("decayMode");
    produces<std::vector<pat::Jet > >("savedJets");
    produces<std::vector<float > >("genPt");
    produces<std::vector<float > > ("dRjetParticle");
    produces<std::vector<float > > ("topSize");

    listOfVars.push_back("et"); 
    listOfVars.push_back("eta"); 
    listOfVars.push_back("mass");
    listOfVars.push_back("SDmass");
    listOfVars.push_back("tau32");
    listOfVars.push_back("tau21");
    listOfVars.push_back("h1_top"); 
    listOfVars.push_back("h2_top"); 
    listOfVars.push_back("h3_top"); 
    listOfVars.push_back("h4_top"); 
    listOfVars.push_back("isotropy_top"); 
    listOfVars.push_back("aplanarity_top"); 
    listOfVars.push_back("sphericity_top"); 
    listOfVars.push_back("thrust_top"); 
    listOfVars.push_back("h1_W"); 
    listOfVars.push_back("h2_W"); 
    listOfVars.push_back("h3_W"); 
    listOfVars.push_back("h4_W"); 
    listOfVars.push_back("isotropy_W"); 
    listOfVars.push_back("aplanarity_W"); 
    listOfVars.push_back("sphericity_W"); 
    listOfVars.push_back("thrust_W"); 
    listOfVars.push_back("h1_Z"); 
    listOfVars.push_back("h2_Z"); 
    listOfVars.push_back("h3_Z"); 
    listOfVars.push_back("h4_Z"); 
    listOfVars.push_back("isotropy_Z"); 
    listOfVars.push_back("aplanarity_Z"); 
    listOfVars.push_back("sphericity_Z"); 
    listOfVars.push_back("thrust_Z"); 
    listOfVars.push_back("h1_H"); 
    listOfVars.push_back("h2_H"); 
    listOfVars.push_back("h3_H"); 
    listOfVars.push_back("h4_H"); 
    listOfVars.push_back("isotropy_H"); 
    listOfVars.push_back("aplanarity_H"); 
    listOfVars.push_back("sphericity_H"); 
    listOfVars.push_back("thrust_H"); 
    listOfVars.push_back("targetX");
    listOfVars.push_back("targetY");
    listOfVars.push_back("NNoutX");
    listOfVars.push_back("NNoutY");
    listOfVars.push_back("Njets_top");
    listOfVars.push_back("Njets_W");
    listOfVars.push_back("Njets_Z");
    listOfVars.push_back("Njets_H");
    listOfVars.push_back("Njets_jet");
    listOfVars.push_back("Njets_orig");
    listOfVars.push_back("bDisc"); 
    listOfVars.push_back("bDisc1"); 
    listOfVars.push_back("bDisc2"); 
    listOfVars.push_back("sumPz_top");
    listOfVars.push_back("sumPz_W");
    listOfVars.push_back("sumPz_Z");
    listOfVars.push_back("sumPz_H");
    listOfVars.push_back("sumPz_jet");
    listOfVars.push_back("sumP_top");
    listOfVars.push_back("sumP_W");
    listOfVars.push_back("sumP_Z");
    listOfVars.push_back("sumP_H");
    listOfVars.push_back("sumP_jet");
    listOfVars.push_back("npv");
    listOfVars.push_back("q");
    listOfVars.push_back("gen_pt");
    listOfVars.push_back("gen_size");
    listOfVars.push_back("dR_gen_jet");
    listOfVars.push_back("et1_jet");
    listOfVars.push_back("et2_jet");
    listOfVars.push_back("et3_jet");
    listOfVars.push_back("et4_jet");
    listOfVars.push_back("m12_jet");
    listOfVars.push_back("m13_jet");
    listOfVars.push_back("m23_jet");
    listOfVars.push_back("m1234_jet");
    listOfVars.push_back("et1_W");
    listOfVars.push_back("et2_W");
    listOfVars.push_back("et3_W");
    listOfVars.push_back("et4_W");
    listOfVars.push_back("m12_W");
    listOfVars.push_back("m13_W");
    listOfVars.push_back("m23_W");
    listOfVars.push_back("m1234_W");
    listOfVars.push_back("et1_Z");
    listOfVars.push_back("et2_Z");
    listOfVars.push_back("et3_Z");
    listOfVars.push_back("et4_Z");
    listOfVars.push_back("m12_Z");
    listOfVars.push_back("m13_Z");
    listOfVars.push_back("m23_Z");
    listOfVars.push_back("m1234_Z");
    listOfVars.push_back("et1_top");
    listOfVars.push_back("et2_top");
    listOfVars.push_back("et3_top");
    listOfVars.push_back("et4_top");
    listOfVars.push_back("m12_top");
    listOfVars.push_back("m13_top");
    listOfVars.push_back("m23_top");
    listOfVars.push_back("m1234_top");
    listOfVars.push_back("et1_H");
    listOfVars.push_back("et2_H");
    listOfVars.push_back("et3_H");
    listOfVars.push_back("et4_H");
    listOfVars.push_back("m12_H");
    listOfVars.push_back("m13_H");
    listOfVars.push_back("m23_H");
    listOfVars.push_back("m1234_H");

    // initialize values
    for (const auto& i : listOfVars)
        treeVars[i] = -999.;


    edm::InputTag pfCandsTag_;
    edm::InputTag ak8JetsTag_;
    edm::InputTag ak4JetsTag_;
    edm::InputTag ak8subjetsTag_;
    edm::InputTag verticesTag_;
    edm::InputTag trigResultsTag_;

    ak8JetsTag_  = edm::InputTag("slimmedJetsAK8", "", "PAT");
    pfCandsTag_  = edm::InputTag("packedPFCandidates", "", "PAT");
    ak4JetsTag_  = edm::InputTag("slimmedJets", "", "PAT");
    verticesTag_ = edm::InputTag("offlineSlimmedPrimaryVertices", "", "PAT");
    ak8subjetsTag_ = edm::InputTag("slimmedJetsAK8PFCHSSoftDropPacked","SubJets", "PAT");

    pfCandsToken_  = consumes<std::vector<pat::PackedCandidate> >(pfCandsTag_);
    ak8JetsToken_  = consumes<std::vector<pat::Jet> >(ak8JetsTag_);
    ak4JetsToken_  = consumes<std::vector<pat::Jet> >(ak4JetsTag_);
    verticesToken_ = consumes<std::vector<reco::Vertex> >(verticesTag_);
    ak8CHSSoftDropSubjetsToken_ = consumes<std::vector<pat::Jet> >( ak8subjetsTag_ );
} // end constructor


BESTProducer::~BESTProducer(){}


void BESTProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    /* Method called to produce data for other modules to use */
    using namespace edm;

    using namespace std;
    using namespace fastjet;
 
    typedef reco::Candidate::PolarLorentzVector fourv;

    // declare all of the values needed for BEST
    std::unique_ptr< std::vector<float > > fw_moments_0( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_1( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_2( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_3( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_4( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPztop( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPzW( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPzZ( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPzH( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPzjet( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPtop( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPW( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPZ( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPH( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sumPjet( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > Njetstop( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > NjetsW( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > NjetsZ( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > NjetsH( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > Njetsjet( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_1_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_2_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_3_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_4_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > isotropy_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sphericity_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > aplanarity_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > thrust_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_1_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_2_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_3_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_4_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > isotropy_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sphericity_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > aplanarity_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > thrust_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_1_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_2_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_3_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_4_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > isotropy_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sphericity_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > aplanarity_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > thrust_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_1_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_2_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_3_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > fw_moments_4_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > isotropy_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > sphericity_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > aplanarity_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > thrust_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > et( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > eta( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > mass( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > SDmass( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > tau32V( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > tau21V( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > bDiscV( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > bDisc1V( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > bDisc2V( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m12_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m23_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m13_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m1234_H( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m12_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m23_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m13_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m1234_Z( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m12_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m23_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m13_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m1234_W( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m12_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m23_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m13_top( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > m1234_top( new std::vector<float>() );


    std::unique_ptr< std::vector<int > > vertV( new std::vector<int>() );
    std::unique_ptr< std::vector<int > > nAK4JetsV( new std::vector<int>() );
    std::unique_ptr< std::vector<float > > qV(new std::vector<float>() );
    std::unique_ptr< std::vector<float > > qsubjet0V(new std::vector<float>() );
    std::unique_ptr< std::vector<float > > qsubjet1V(new std::vector<float>() );
    std::unique_ptr< std::vector<int > > decayModeV( new std::vector<int>() ) ;
    std::unique_ptr< std::vector<pat::Jet > > savedJetsV( new std::vector<pat::Jet>() );

    std::unique_ptr< std::vector<float > > genPt(new std::vector<float>() );
    std::unique_ptr< std::vector<float > > dRjetParticle( new std::vector<float>() );
    std::unique_ptr< std::vector<float > > topSize(new std::vector<float>() );

    // Access all of the necessary information from the data 
    Handle< std::vector<pat::PackedCandidate> > pfCands;
    iEvent.getByToken(pfCandsToken_, pfCands);

    Handle< std::vector<pat::Jet> > ak8Jets;
    iEvent.getByToken(ak8JetsToken_, ak8Jets);

    Handle< std::vector<pat::Jet> > ak4Jets;
    iEvent.getByToken(ak4JetsToken_, ak4Jets);

    Handle< std::vector<pat::Jet> > ak8Subjets;
    iEvent.getByToken(ak8CHSSoftDropSubjetsToken_, ak8Subjets);

    Handle< std::vector<reco::Vertex> > vertices;
    iEvent.getByToken(verticesToken_, vertices);

    vertV->push_back(vertices->size() );
    treeVars["npv"] = vertices->size();


    int nAK4Jets(0);
    for (std::vector<pat::Jet>::const_iterator jetBegin = ak4Jets->begin(), jetEnd = ak4Jets->end(), ijet = jetBegin; ijet != jetEnd; ++ijet){
        if (ijet->pt() < 30 || abs( ijet->eta() ) > 2.4) continue;
        nAK4Jets++;
    }
    nAK4JetsV->push_back(nAK4Jets);
    std::string userFloat_tau1 = (m_year==2016) ? "NjettinessAK8:tau1" : "ak8PFJetsCHSValueMap:NjettinessAK8CHSTau1";
    std::string userFloat_tau2 = (m_year==2016) ? "NjettinessAK8:tau2" : "ak8PFJetsCHSValueMap:NjettinessAK8CHSTau2";
    std::string userFloat_tau3 = (m_year==2016) ? "NjettinessAK8:tau3" : "ak8PFJetsCHSValueMap:NjettinessAK8CHSTau3";

    int nJets = 0;
    for (std::vector<pat::Jet>::const_iterator jetBegin = ak8Jets->begin(), jetEnd = ak8Jets->end(), ijet = jetBegin; ijet != jetEnd; ++ijet){
        nJets++;

        auto const & thisSubjets = ijet->subjets();

        // Quality cut for BEST
        // pT > 350 (400 GeV used in analysis)
        // >= 2 softdrop subjets
        if (ijet->pt() < 350) continue;
        if (thisSubjets.size() < 2 || ijet->numberOfDaughters() < 2) continue;

        float btagValue1 = thisSubjets.at(0)->bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags");
        float btagValue2 = thisSubjets.at(1)->bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags");

        treeVars["bDisc"]  = max( btagValue1, btagValue2 );
        treeVars["bDisc1"] = btagValue1;
        treeVars["bDisc2"] = btagValue2;

        float tau3 = ijet->userFloat(userFloat_tau3);
        float tau2 = ijet->userFloat(userFloat_tau2);
        float tau1 = ijet->userFloat(userFloat_tau1);
        float tau32 = tau3/tau2;
        float tau21 = tau2/tau1;

        fourv thisJet      = ijet->polarP4();
        treeVars["et"]     = thisJet.Pt();
        treeVars["eta"]    = thisJet.Rapidity();
        treeVars["mass"]   = thisJet.M();
        treeVars["SDmass"] = (m_year==2016) ? ijet->userFloat("ak8PFJetsCHSSoftDropMass") : ijet->userFloat("ak8PFJetsCHSValueMap:ak8PFJetsCHSSoftDropMass");

        treeVars["tau32"] = tau32;
        treeVars["tau21"] = tau21;

        treeVars["targetX"] = NNtargetX_;
        treeVars["targetY"] = NNtargetY_;

        TLorentzVector thisJetLV;
        TLorentzVector thisJetLV_W;
        TLorentzVector thisJetLV_Z;
        TLorentzVector thisJetLV_H;
        TLorentzVector thisJetLV_top;
        thisJetLV.SetPtEtaPhiM(thisJet.Pt(), thisJet.Eta(), thisJet.Phi(), thisJet.M() );
        thisJetLV_W.SetPtEtaPhiM(thisJet.Pt(), thisJet.Eta(), thisJet.Phi(), 80.4 );
        thisJetLV_Z.SetPtEtaPhiM(thisJet.Pt(), thisJet.Eta(), thisJet.Phi(), 91.2 );
        thisJetLV_H.SetPtEtaPhiM(thisJet.Pt(), thisJet.Eta(), thisJet.Phi(), 125. );
        thisJetLV_top.SetPtEtaPhiM(thisJet.Pt(), thisJet.Eta(), thisJet.Phi(), 172.5 );

        std::vector<TLorentzVector> particles_jet;
        std::vector<TLorentzVector> particles_top;
        std::vector<TLorentzVector> particles_W;
        std::vector<TLorentzVector> particles_Z;
        std::vector<TLorentzVector> particles_H;

        std::vector<math::XYZVector> particles2_jet;
        std::vector<math::XYZVector> particles2_top;
        std::vector<math::XYZVector> particles2_W;
        std::vector<math::XYZVector> particles2_Z;
        std::vector<math::XYZVector> particles2_H;

        std::vector<reco::LeafCandidate> particles3_jet;
        std::vector<reco::LeafCandidate> particles3_top;
        std::vector<reco::LeafCandidate> particles3_W;
        std::vector<reco::LeafCandidate> particles3_Z;
        std::vector<reco::LeafCandidate> particles3_H;

        vector<fastjet::PseudoJet> topFJparticles;
        vector<fastjet::PseudoJet> ZFJparticles;
        vector<fastjet::PseudoJet> WFJparticles;
        vector<fastjet::PseudoJet> HFJparticles;
        vector<fastjet::PseudoJet> topFJparticles_noBoost;
    
        vector<fastjet::PseudoJet> jetFJparticles;
        vector<fastjet::PseudoJet> jetFJparticles_noBoost;
        vector<fastjet::PseudoJet> jetFJparticles_transformed;
    
        TVector3 transformedV;

        // Jet Charge
        float qxptsum = 0.0;
        float ptsum = pow(ijet->pt(), kappa_);

        float qxptsum_0 = 0.0;
        float ptsum_0 = pow(ijet->daughter(0)->pt(), kappa_);
        float qxptsum_1 = 0.0;
        float ptsum_1 = pow(ijet->daughter(1)->pt(), kappa_);

        vector<reco::Candidate * > daughtersOfJet;

        for (unsigned int i = 0; i < ijet->daughter(0)->numberOfDaughters(); i++){
            daughtersOfJet.push_back( (reco::Candidate *) ijet->daughter(0)->daughter(i) );
            if (ijet->daughter(0)->daughter(i)->pt() > 1.0) qxptsum_0 += ijet->daughter(0)->daughter(i)->charge() * pow( ijet->daughter(0)->daughter(i)->pt(), kappa_);
        }
        qsubjet0V->push_back(qxptsum_0 /ptsum_0);

        for (unsigned int i = 0; i < ijet->daughter(1)->numberOfDaughters(); i++){
            daughtersOfJet.push_back( (reco::Candidate *) ijet->daughter(1)->daughter(i));
            if (ijet->daughter(1)->daughter(i)->pt() > 1.0) qxptsum_1 += ijet->daughter(1)->daughter(i)->charge() * pow( ijet->daughter(1)->daughter(i)->pt(), kappa_);
        }
        qsubjet1V->push_back(qxptsum_1 /ptsum_1);

        for (unsigned int i = 2; i< ijet->numberOfDaughters(); i++){
            daughtersOfJet.push_back( (reco::Candidate *) ijet->daughter(i) );
        }

        // loop over all of the jet constituents (for BEST & jet charge)
        for(unsigned int i = 0; i < daughtersOfJet.size(); i++){

            TVector3 ijetVect( ijet->px(), ijet->py(), ijet->pz() );

            // Quality cut on the constituent
            if (daughtersOfJet[i]->pt() < 0.5) continue;

            if (daughtersOfJet[i]->pt() > 1.0) qxptsum += daughtersOfJet[i]->charge() * pow( daughtersOfJet[i]->pt(), kappa_);//(ijetVect) ;


            TLorentzVector thisParticleLV_jet( daughtersOfJet[i]->px(), daughtersOfJet[i]->py(), daughtersOfJet[i]->pz(), daughtersOfJet[i]->energy() );        
            TLorentzVector thisParticleLV_top( daughtersOfJet[i]->px(), daughtersOfJet[i]->py(), daughtersOfJet[i]->pz(), daughtersOfJet[i]->energy() );        
            TLorentzVector thisParticleLV_W( daughtersOfJet[i]->px(), daughtersOfJet[i]->py(), daughtersOfJet[i]->pz(), daughtersOfJet[i]->energy() );        
            TLorentzVector thisParticleLV_Z( daughtersOfJet[i]->px(), daughtersOfJet[i]->py(), daughtersOfJet[i]->pz(), daughtersOfJet[i]->energy() );        
            TLorentzVector thisParticleLV_H( daughtersOfJet[i]->px(), daughtersOfJet[i]->py(), daughtersOfJet[i]->pz(), daughtersOfJet[i]->energy() );        

            topFJparticles_noBoost.push_back( PseudoJet( thisParticleLV_top.X(), thisParticleLV_top.Y(), thisParticleLV_top.Z(), thisParticleLV_top.T() ) );
            jetFJparticles_noBoost.push_back( PseudoJet( thisParticleLV_jet.X(), thisParticleLV_jet.Y(), thisParticleLV_jet.Z(), thisParticleLV_jet.T() ) );

            TLorentzVector thisParticleLV_transformed( transformedV.X(), transformedV.Y(), transformedV.Z(), thisParticleLV_jet.E() );
            jetFJparticles_transformed.push_back( PseudoJet( thisParticleLV_transformed.X(), thisParticleLV_transformed.Y(), thisParticleLV_transformed.Z(), thisParticleLV_transformed.T() ) );    

            thisParticleLV_jet.Boost( -thisJetLV.BoostVector() );
            thisParticleLV_Z.Boost( -thisJetLV_Z.BoostVector() );
            thisParticleLV_W.Boost( -thisJetLV_W.BoostVector() );
            thisParticleLV_H.Boost( -thisJetLV_H.BoostVector() );
            thisParticleLV_top.Boost( -thisJetLV_top.BoostVector() );

            pboost( thisJetLV_W.Vect(), thisParticleLV_W.Vect(), thisParticleLV_W);
            pboost( thisJetLV_Z.Vect(), thisParticleLV_Z.Vect(), thisParticleLV_Z);
            pboost( thisJetLV_top.Vect(), thisParticleLV_top.Vect(), thisParticleLV_top);
            pboost( thisJetLV_H.Vect(), thisParticleLV_H.Vect(), thisParticleLV_H);

            particles_jet.push_back( thisParticleLV_jet );
            particles_top.push_back( thisParticleLV_top );
            particles_W.push_back( thisParticleLV_W );
            particles_Z.push_back( thisParticleLV_Z );
            particles_H.push_back( thisParticleLV_H );

            topFJparticles.push_back( PseudoJet( thisParticleLV_top.X(), thisParticleLV_top.Y(), thisParticleLV_top.Z(), thisParticleLV_top.T() ) );
            WFJparticles.push_back( PseudoJet( thisParticleLV_W.X(), thisParticleLV_W.Y(), thisParticleLV_W.Z(), thisParticleLV_W.T() ) );
            ZFJparticles.push_back( PseudoJet( thisParticleLV_Z.X(), thisParticleLV_Z.Y(), thisParticleLV_Z.Z(), thisParticleLV_Z.T() ) );
            HFJparticles.push_back( PseudoJet( thisParticleLV_H.X(), thisParticleLV_H.Y(), thisParticleLV_H.Z(), thisParticleLV_H.T() ) );
            jetFJparticles.push_back( PseudoJet( thisParticleLV_jet.X(), thisParticleLV_jet.Y(), thisParticleLV_jet.Z(), thisParticleLV_jet.T() ) );

            particles2_top.push_back( math::XYZVector( thisParticleLV_top.X(), thisParticleLV_top.Y(), thisParticleLV_top.Z() ));
            particles3_top.push_back( reco::LeafCandidate(+1, reco::Candidate::LorentzVector( thisParticleLV_top.X(), thisParticleLV_top.Y(), thisParticleLV_top.Z(), thisParticleLV_top.T()     ) ));
            particles2_W.push_back( math::XYZVector( thisParticleLV_W.X(), thisParticleLV_W.Y(), thisParticleLV_W.Z() ));
            particles3_W.push_back( reco::LeafCandidate(+1, reco::Candidate::LorentzVector( thisParticleLV_W.X(), thisParticleLV_W.Y(), thisParticleLV_W.Z(), thisParticleLV_W.T()     ) ));
            particles2_Z.push_back( math::XYZVector( thisParticleLV_Z.X(), thisParticleLV_Z.Y(), thisParticleLV_Z.Z() ));
            particles3_Z.push_back( reco::LeafCandidate(+1, reco::Candidate::LorentzVector( thisParticleLV_Z.X(), thisParticleLV_Z.Y(), thisParticleLV_Z.Z(), thisParticleLV_Z.T()     ) ));
            particles2_H.push_back( math::XYZVector( thisParticleLV_H.X(), thisParticleLV_H.Y(), thisParticleLV_H.Z() ));
            particles3_H.push_back( reco::LeafCandidate(+1, reco::Candidate::LorentzVector( thisParticleLV_H.X(), thisParticleLV_H.Y(), thisParticleLV_H.Z(), thisParticleLV_H.T()     ) ));
        } // end loop over daughters

        float jetq = qxptsum / ptsum;  // jet charge
        qV->push_back(jetq);

        double fwm[5] = { 0.0, 0.0 ,0.0 ,0.0,0.0};
        double fwm_W[5] = { 0.0, 0.0 ,0.0 ,0.0,0.0};
        double fwm_top[5] = { 0.0, 0.0 ,0.0 ,0.0,0.0};
        double fwm_Z[5] = { 0.0, 0.0 ,0.0 ,0.0,0.0};
        double fwm_H[5] = { 0.0, 0.0 ,0.0 ,0.0,0.0};
        FWMoments( particles_W, fwm_W);
        FWMoments( particles_jet, fwm);
        FWMoments( particles_top, fwm_top);
        FWMoments( particles_Z, fwm_Z);
        FWMoments( particles_H, fwm_H);

        EventShapeVariables eventShapes_top( particles2_top );
        EventShapeVariables eventShapes_W( particles2_W );
        EventShapeVariables eventShapes_Z( particles2_Z );
        EventShapeVariables eventShapes_H( particles2_H );
        Thrust thrustCalculator_top( particles3_top.begin(), particles3_top.end() );
        Thrust thrustCalculator_W( particles3_W.begin(), particles3_W.end() );
        Thrust thrustCalculator_Z( particles3_Z.begin(), particles3_Z.end() );
        Thrust thrustCalculator_H( particles3_H.begin(), particles3_H.end() );

        TVectorD eigenValues(3);

        JetDefinition jet_def(antikt_algorithm, AK4_radius_);
        JetDefinition jet_def2(antikt_algorithm, AK8_radius_);

        ClusterSequence cs(topFJparticles, jet_def);
        ClusterSequence cs_W(WFJparticles, jet_def);
        ClusterSequence cs_Z(ZFJparticles, jet_def);
        ClusterSequence cs_H(HFJparticles, jet_def);
        ClusterSequence cs_jet(jetFJparticles, jet_def);
        ClusterSequence cs_noBoost(topFJparticles_noBoost, jet_def2);
       
        ClusterSequence cs_transformed(jetFJparticles_transformed, jet_def);

 
        vector<PseudoJet> jetsFJ = sorted_by_pt(cs.inclusive_jets(20.0));
        vector<PseudoJet> jetsFJ_W = sorted_by_pt(cs_W.inclusive_jets(20.0));
        vector<PseudoJet> jetsFJ_Z = sorted_by_pt(cs_Z.inclusive_jets(20.0));
        vector<PseudoJet> jetsFJ_H = sorted_by_pt(cs_H.inclusive_jets(20.0));
        vector<PseudoJet> jetsFJ_jet = sorted_by_pt(cs_jet.inclusive_jets(20.0));
        vector<PseudoJet> jetsFJ_noBoost = sorted_by_pt(cs_noBoost.inclusive_jets(20.0));

        vector<PseudoJet> jetsFJ_transformed = cs_transformed.inclusive_jets(20.0);


        //sum of jet pz and p  Indices = top, W, Z, H, j
        float sumP[5] = {0.0};
        float sumPz[5] = {0.0};

        TLorentzVector m1234LV_jet(0.,0.,0.,0.);
        TLorentzVector m1234LV_W(0.,0.,0.,0.);
        TLorentzVector m1234LV_Z(0.,0.,0.,0.);
        TLorentzVector m1234LV_H(0.,0.,0.,0.);
        TLorentzVector m1234LV_top(0.,0.,0.,0.);
        TLorentzVector m12LV_jet(0.,0.,0.,0.);
        TLorentzVector m12LV_W(0.,0.,0.,0.);
        TLorentzVector m12LV_Z(0.,0.,0.,0.);
        TLorentzVector m12LV_H(0.,0.,0.,0.);
        TLorentzVector m12LV_top(0.,0.,0.,0.);
        TLorentzVector m13LV_jet(0.,0.,0.,0.);
        TLorentzVector m13LV_W(0.,0.,0.,0.);
        TLorentzVector m13LV_Z(0.,0.,0.,0.);
        TLorentzVector m13LV_H(0.,0.,0.,0.);
        TLorentzVector m13LV_top(0.,0.,0.,0.);
        TLorentzVector m23LV_jet(0.,0.,0.,0.);
        TLorentzVector m23LV_W(0.,0.,0.,0.);
        TLorentzVector m23LV_Z(0.,0.,0.,0.);
        TLorentzVector m23LV_H(0.,0.,0.,0.);
        TLorentzVector m23LV_top(0.,0.,0.,0.);

        size_t maxJets = 4;

        for (size_t i=0, size=TMath::Min(maxJets, jetsFJ.size()); i<size; i++){
            sumPz[0] += jetsFJ[i].pz();
            sumP[0]  += sqrt( jetsFJ[i].modp2() );
            thisJetLV = TLorentzVector(jetsFJ[i].px(), jetsFJ[i].py(), jetsFJ[i].pz(), jetsFJ[i].e());
            m1234LV_top += thisJetLV;
            switch (i){
                case 0:
                    treeVars["et1_top"] = jetsFJ[i].pt();
                    m12LV_top += thisJetLV;
                    m13LV_top += thisJetLV;
                    break;
                case 1:
                    treeVars["et2_top"] = jetsFJ[i].pt();
                    m12LV_top += thisJetLV;
                    m23LV_top += thisJetLV;
                    break;
                case 2:
                    treeVars["et3_top"] = jetsFJ[i].pt();
                    m13LV_top += thisJetLV;
                    m23LV_top += thisJetLV;
                    break;
                case 3:
                    treeVars["et4_top"] = jetsFJ[i].pt();
                    break;
            }
        } // end jetsFJ top
        for (size_t i=0; i < TMath::Min(maxJets, jetsFJ_W.size()); i++){
            sumPz[1] += jetsFJ_W[i].pz();
            sumP[1] += sqrt( jetsFJ_W[i].modp2() );
            thisJetLV = TLorentzVector(jetsFJ_W[i].px(), jetsFJ_W[i].py(), jetsFJ_W[i].pz(), jetsFJ_W[i].e());
            m1234LV_W += thisJetLV;
            switch (i){
                case 0:
                    treeVars["et1_W"] = jetsFJ_W[i].pt();
                    m12LV_W += thisJetLV;
                    m13LV_W += thisJetLV;
                    break;
                case 1:
                    treeVars["et2_W"] = jetsFJ_W[i].pt();
                    m12LV_W += thisJetLV;
                    m23LV_W += thisJetLV;
                    break;
                case 2:
                    treeVars["et3_W"] = jetsFJ_W[i].pt();
                    m13LV_W += thisJetLV;
                    m23LV_W += thisJetLV;
                    break;
                case 3:
                    treeVars["et4_W"] = jetsFJ_W[i].pt();
                    break;
            }
        } // end jetsFJ_W
        for (size_t i=0; i < TMath::Min(maxJets, jetsFJ_Z.size()); i++){
            sumPz[2] += jetsFJ_Z[i].pz();
            sumP[2] += sqrt( jetsFJ_Z[i].modp2() );
            thisJetLV = TLorentzVector(jetsFJ_Z[i].px(), jetsFJ_Z[i].py(), jetsFJ_Z[i].pz(), jetsFJ_Z[i].e());
            m1234LV_Z += thisJetLV;
            switch (i){
                case 0:
                    treeVars["et1_Z"] = jetsFJ_Z[i].pt();
                    m12LV_Z += thisJetLV;
                    m13LV_Z += thisJetLV;
                    break;
                case 1:
                    treeVars["et2_Z"] = jetsFJ_Z[i].pt();
                    m12LV_Z += thisJetLV;
                    m23LV_Z += thisJetLV;
                    break;
                case 2:
                    treeVars["et3_Z"] = jetsFJ_Z[i].pt();
                    m13LV_Z += thisJetLV;
                    m23LV_Z += thisJetLV;
                    break;
                case 3:
                    treeVars["et4_Z"] = jetsFJ_Z[i].pt();
                    break;
            }
        } // end jetsFJ_Z
        for (size_t i=0; i < TMath::Min(maxJets, jetsFJ_H.size()); i++){
            sumPz[3] += jetsFJ_H[i].pz();
            sumP[3] += sqrt( jetsFJ_H[i].modp2() );
            thisJetLV = TLorentzVector(jetsFJ_H[i].px(), jetsFJ_H[i].py(), jetsFJ_H[i].pz(), jetsFJ_H[i].e());
            m1234LV_H += thisJetLV;
            switch (i){
                case 0:
                    treeVars["et1_H"] = jetsFJ_H[i].pt();
                    m12LV_H += thisJetLV;
                    m13LV_H += thisJetLV;
                    break;
                case 1:
                    treeVars["et2_H"] = jetsFJ_H[i].pt();
                    m12LV_H += thisJetLV;
                    m23LV_H += thisJetLV;
                    break;
                case 2:
                    treeVars["et3_H"] = jetsFJ_H[i].pt();
                    m13LV_H += thisJetLV;
                    m23LV_H += thisJetLV;
                    break;
                case 3:
                    treeVars["et4_H"] = jetsFJ_H[i].pt();
                    break;
            }
        } // end jetsFJ_H
        for (size_t i=0; i < TMath::Min(maxJets, jetsFJ_jet.size()); i++){
            sumPz[4] += jetsFJ_jet[i].pz();
            sumP[4] += sqrt( jetsFJ_jet[i].modp2() );
            thisJetLV = TLorentzVector(jetsFJ_jet[i].px(), jetsFJ_jet[i].py(), jetsFJ_jet[i].pz(), jetsFJ_jet[i].e());
            m1234LV_jet += thisJetLV;
            switch (i){
                case 0:
                    treeVars["et1_jet"] = jetsFJ_jet[i].pt();
                    m12LV_jet += thisJetLV;
                    m13LV_jet += thisJetLV;
                    break;
                case 1:
                    treeVars["et2_jet"] = jetsFJ_jet[i].pt();
                    m12LV_jet += thisJetLV;
                    m23LV_jet += thisJetLV;
                    break;
                case 2:
                    treeVars["et3_jet"] = jetsFJ_jet[i].pt();
                    m13LV_jet += thisJetLV;
                    m23LV_jet += thisJetLV;
                    break;
                case 3:
                    treeVars["et4_jet"] = jetsFJ_jet[i].pt();
                    break;
            }
        } // end jetsFJ_jet



        treeVars["m1234_jet"] = m1234LV_jet.M();
        treeVars["m12_jet"] = m12LV_jet.M();
        treeVars["m23_jet"] = m23LV_jet.M();
        treeVars["m13_jet"] = m13LV_jet.M();
        treeVars["m1234_top"] = m1234LV_top.M();
        treeVars["m12_top"] = m12LV_top.M();
        treeVars["m23_top"] = m23LV_top.M();
        treeVars["m13_top"] = m13LV_top.M();
        treeVars["m1234_W"] = m1234LV_W.M();
        treeVars["m12_W"] = m12LV_W.M();
        treeVars["m23_W"] = m23LV_W.M();
        treeVars["m13_W"] = m13LV_W.M();
        treeVars["m1234_Z"] = m1234LV_Z.M();
        treeVars["m12_Z"] = m12LV_Z.M();
        treeVars["m23_Z"] = m23LV_Z.M();
        treeVars["m13_Z"] = m13LV_Z.M();
        treeVars["m1234_H"] = m1234LV_H.M();
        treeVars["m12_H"] = m12LV_H.M();
        treeVars["m23_H"] = m23LV_H.M();
        treeVars["m13_H"] = m13LV_H.M();
        treeVars["q"]     = jetq;
        treeVars["sumPz_top"] = sumPz[0];
        treeVars["sumPz_W"] = sumPz[1];
        treeVars["sumPz_Z"] = sumPz[2];
        treeVars["sumPz_H"] = sumPz[3];
        treeVars["sumPz_jet"] = sumPz[4];
        treeVars["sumP_top"] = sumP[0];
        treeVars["sumP_W"] = sumP[1];
        treeVars["sumP_Z"] = sumP[2];
        treeVars["sumP_H"] = sumP[3];
        treeVars["sumP_jet"] = sumP[4];

        treeVars["Njets_top"] = jetsFJ.size();
        treeVars["Njets_W"] = jetsFJ_W.size();
        treeVars["Njets_Z"] = jetsFJ_Z.size();
        treeVars["Njets_H"] = jetsFJ_H.size();
        treeVars["Njets_jet"] = jetsFJ_jet.size();
        treeVars["Njets_orig"] = jetsFJ_noBoost.size();

        fw_moments_0->push_back(fwm[0]);    
        fw_moments_1->push_back(fwm[1]);    
        fw_moments_2->push_back(fwm[2]);    
        fw_moments_3->push_back(fwm[3]);    
        fw_moments_4->push_back(fwm[4]);    

        treeVars["h1_top"] = fwm_top[1];
        treeVars["h2_top"] = fwm_top[2];
        treeVars["h3_top"] = fwm_top[3];
        treeVars["h4_top"] = fwm_top[4];
        treeVars["isotropy_top"]   = eventShapes_top.isotropy(); 
        treeVars["sphericity_top"] = eventShapes_top.sphericity(2);
        treeVars["aplanarity_top"] = eventShapes_top.aplanarity(2);
        treeVars["thrust_top"]     = thrustCalculator_top.thrust();
        treeVars["h1_W"] = fwm_W[1];
        treeVars["h2_W"] = fwm_W[2];
        treeVars["h3_W"] = fwm_W[3];
        treeVars["h4_W"] = fwm_W[4];
        treeVars["isotropy_W"]   = eventShapes_W.isotropy(); 
        treeVars["sphericity_W"] = eventShapes_W.sphericity(2);
        treeVars["aplanarity_W"] = eventShapes_W.aplanarity(2);
        treeVars["thrust_W"]     = thrustCalculator_W.thrust();
        treeVars["h1_Z"] = fwm_Z[1];
        treeVars["h2_Z"] = fwm_Z[2];
        treeVars["h3_Z"] = fwm_Z[3];
        treeVars["h4_Z"] = fwm_Z[4];
        treeVars["isotropy_Z"]   = eventShapes_Z.isotropy(); 
        treeVars["sphericity_Z"] = eventShapes_Z.sphericity(2);
        treeVars["aplanarity_Z"] = eventShapes_Z.aplanarity(2);
        treeVars["thrust_Z"]     = thrustCalculator_Z.thrust();
        treeVars["h1_H"] = fwm_H[1];
        treeVars["h2_H"] = fwm_H[2];
        treeVars["h3_H"] = fwm_H[3];
        treeVars["h4_H"] = fwm_H[4];
        treeVars["isotropy_H"]   = eventShapes_H.isotropy(); 
        treeVars["sphericity_H"] = eventShapes_H.sphericity(2);
        treeVars["aplanarity_H"] = eventShapes_H.aplanarity(2);
        treeVars["thrust_H"]     = thrustCalculator_H.thrust();

        sumPztop->push_back( treeVars["sumPz_top"]);
        sumPzW->push_back( treeVars["sumPz_W"]);
        sumPzZ->push_back( treeVars["sumPz_Z"]);
        sumPzH->push_back( treeVars["sumPz_H"]);
        sumPzjet->push_back( treeVars["sumPz_jet"]);
        sumPtop->push_back( treeVars["sumP_top"]);
        sumPW->push_back( treeVars["sumP_W"]);
        sumPZ->push_back( treeVars["sumP_Z"]);
        sumPH->push_back( treeVars["sumP_H"]);
        sumPjet->push_back( treeVars["sumP_jet"]);
        Njetstop->push_back( treeVars["Njets_top"]);
        NjetsW->push_back( treeVars["Njets_W"]);
        NjetsZ->push_back( treeVars["Njets_Z"]);
        NjetsH->push_back( treeVars["Njets_H"]);
        Njetsjet->push_back( treeVars["Njets_jet"]);
        fw_moments_1_top->push_back( treeVars["h1_top"]);
        fw_moments_2_top->push_back( treeVars["h2_top"]);
        fw_moments_3_top->push_back( treeVars["h3_top"]);
        fw_moments_4_top->push_back( treeVars["h4_top"]);
        isotropy_top->push_back( treeVars["isotropy_top"]);
        sphericity_top->push_back( treeVars["sphericity_top"]);
        aplanarity_top->push_back( treeVars["aplanarity_top"]);
        thrust_top->push_back( treeVars["thrust_top"]);
        fw_moments_1_W->push_back( treeVars["h1_W"]);
        fw_moments_2_W->push_back( treeVars["h2_W"]);
        fw_moments_3_W->push_back( treeVars["h3_W"]);
        fw_moments_4_W->push_back( treeVars["h4_W"]);
        isotropy_W->push_back( treeVars["isotropy_W"]);
        sphericity_W->push_back( treeVars["sphericity_W"]);
        aplanarity_W->push_back( treeVars["aplanarity_W"]);
        thrust_W->push_back( treeVars["thrust_W"]);
        fw_moments_1_Z->push_back( treeVars["h1_Z"]);
        fw_moments_2_Z->push_back( treeVars["h2_Z"]);
        fw_moments_3_Z->push_back( treeVars["h3_Z"]);
        fw_moments_4_Z->push_back( treeVars["h4_Z"]);
        isotropy_Z->push_back( treeVars["isotropy_Z"]);
        sphericity_Z->push_back( treeVars["sphericity_Z"]);
        aplanarity_Z->push_back( treeVars["aplanarity_Z"]);
        thrust_Z->push_back( treeVars["thrust_Z"]);
        fw_moments_1_H->push_back( treeVars["h1_H"]);
        fw_moments_2_H->push_back( treeVars["h2_H"]);
        fw_moments_3_H->push_back( treeVars["h3_H"]);
        fw_moments_4_H->push_back( treeVars["h4_H"]);
        isotropy_H->push_back( treeVars["isotropy_H"]);
        sphericity_H->push_back( treeVars["sphericity_H"]);
        aplanarity_H->push_back( treeVars["aplanarity_H"]);
        thrust_H->push_back( treeVars["thrust_H"]);
        et->push_back( treeVars["et"]);
        eta->push_back( treeVars["eta"]);
        mass->push_back( treeVars["mass"]);
        SDmass->push_back( treeVars["SDmass"]);
        tau32V->push_back( treeVars["tau32"]);
        tau21V->push_back( treeVars["tau21"]);
        bDiscV->push_back( treeVars["bDisc"]);
        bDisc1V->push_back( treeVars["bDisc1"]);
        bDisc2V->push_back( treeVars["bDisc2"]);
        m12_H->push_back( treeVars["m12_H"]);
        m23_H->push_back( treeVars["m23_H"]);
        m13_H->push_back( treeVars["m13_H"]);
        m1234_H->push_back( treeVars["m1234_H"]);
        m12_W->push_back( treeVars["m12_W"]);
        m23_W->push_back( treeVars["m23_W"]);
        m13_W->push_back( treeVars["m13_W"]);
        m1234_W->push_back( treeVars["m1234_W"]);
        m12_Z->push_back( treeVars["m12_Z"]);
        m23_Z->push_back( treeVars["m23_Z"]);
        m13_Z->push_back( treeVars["m13_Z"]);
        m1234_Z->push_back( treeVars["m1234_Z"]);
        m12_top->push_back( treeVars["m12_top"]);
        m23_top->push_back( treeVars["m23_top"]);
        m13_top->push_back( treeVars["m13_top"]);
        m1234_top->push_back( treeVars["m1234_top"]);

        savedJetsV->push_back( *ijet );
    } //End AK8 loop



    iEvent.put( std::move( fw_moments_0), "FWmoment0");
    iEvent.put( std::move( fw_moments_1), "FWmoment1");
    iEvent.put( std::move( fw_moments_2), "FWmoment2");
    iEvent.put( std::move( fw_moments_3), "FWmoment3");
    iEvent.put( std::move( fw_moments_4), "FWmoment4");
    iEvent.put( std::move( sumPztop), "sumPztop");
    iEvent.put( std::move( sumPzW), "sumPzW");
    iEvent.put( std::move( sumPzZ), "sumPzZ");
    iEvent.put( std::move( sumPzH), "sumPzH");
    iEvent.put( std::move( sumPzjet), "sumPzjet");
    iEvent.put( std::move( sumPtop), "sumPtop");
    iEvent.put( std::move( sumPW), "sumPW");
    iEvent.put( std::move( sumPZ), "sumPZ");
    iEvent.put( std::move( sumPH), "sumPH");
    iEvent.put( std::move( sumPjet), "sumPjet");
    iEvent.put( std::move( Njetstop), "Njetstop");
    iEvent.put( std::move( NjetsW), "NjetsW");
    iEvent.put( std::move( NjetsZ), "NjetsZ");
    iEvent.put( std::move( NjetsH), "NjetsH");
    iEvent.put( std::move( Njetsjet), "Njetsjet");
    iEvent.put( std::move( fw_moments_1_top), "FWmoment1top");
    iEvent.put( std::move( fw_moments_2_top), "FWmoment2top");
    iEvent.put( std::move( fw_moments_3_top), "FWmoment3top");
    iEvent.put( std::move( fw_moments_4_top), "FWmoment4top");
    iEvent.put( std::move( isotropy_top), "isotropytop");
    iEvent.put( std::move( sphericity_top), "sphericitytop");
    iEvent.put( std::move( aplanarity_top), "aplanaritytop");
    iEvent.put( std::move( thrust_top), "thrusttop");
    iEvent.put( std::move( fw_moments_1_W), "FWmoment1W");
    iEvent.put( std::move( fw_moments_2_W), "FWmoment2W");
    iEvent.put( std::move( fw_moments_3_W), "FWmoment3W");
    iEvent.put( std::move( fw_moments_4_W), "FWmoment4W");
    iEvent.put( std::move( isotropy_W), "isotropyW");
    iEvent.put( std::move( sphericity_W), "sphericityW");
    iEvent.put( std::move( aplanarity_W), "aplanarityW");
    iEvent.put( std::move( thrust_W), "thrustW");
    iEvent.put( std::move( fw_moments_1_Z), "FWmoment1Z");
    iEvent.put( std::move( fw_moments_2_Z), "FWmoment2Z");
    iEvent.put( std::move( fw_moments_3_Z), "FWmoment3Z");
    iEvent.put( std::move( fw_moments_4_Z), "FWmoment4Z");
    iEvent.put( std::move( isotropy_Z), "isotropyZ");
    iEvent.put( std::move( sphericity_Z), "sphericityZ");
    iEvent.put( std::move( aplanarity_Z), "aplanarityZ");
    iEvent.put( std::move( thrust_Z), "thrustZ");
    iEvent.put( std::move( fw_moments_1_H), "FWmoment1H");
    iEvent.put( std::move( fw_moments_2_H), "FWmoment2H");
    iEvent.put( std::move( fw_moments_3_H), "FWmoment3H");
    iEvent.put( std::move( fw_moments_4_H), "FWmoment4H");
    iEvent.put( std::move( isotropy_H), "isotropyH");
    iEvent.put( std::move( sphericity_H), "sphericityH");
    iEvent.put( std::move( aplanarity_H), "aplanarityH");
    iEvent.put( std::move( thrust_H), "thrustH");
    iEvent.put( std::move( et), "et");
    iEvent.put( std::move( eta), "eta");
    iEvent.put( std::move( mass), "mass");
    iEvent.put( std::move( SDmass), "SDmass");
    iEvent.put( std::move( tau32V), "tau32");
    iEvent.put( std::move( tau21V), "tau21");
    iEvent.put( std::move( bDiscV), "bDisc");
    iEvent.put( std::move( bDisc1V), "bDisc1");
    iEvent.put( std::move( bDisc2V), "bDisc2");
    iEvent.put( std::move( vertV), "nPV");
    iEvent.put( std::move( decayModeV), "decayMode");
    iEvent.put( std::move( nAK4JetsV), "nAK4Jets");
    iEvent.put( std::move( savedJetsV), "savedJets");
    iEvent.put( std::move( qV), "q");
    iEvent.put( std::move( qsubjet0V), "qsubjet0");
    iEvent.put( std::move( qsubjet1V), "qsubjet1");
    iEvent.put( std::move( m12_H), "m12H"); 
    iEvent.put( std::move( m23_H), "m23H"); 
    iEvent.put( std::move( m13_H), "m13H"); 
    iEvent.put( std::move( m1234_H), "m1234H"); 
    iEvent.put( std::move( m12_W), "m12W"); 
    iEvent.put( std::move( m23_W), "m23W"); 
    iEvent.put( std::move( m13_W), "m13W"); 
    iEvent.put( std::move( m1234_W), "m1234W"); 
    iEvent.put( std::move( m12_Z), "m12Z"); 
    iEvent.put( std::move( m23_Z), "m23Z"); 
    iEvent.put( std::move( m13_Z), "m13Z"); 
    iEvent.put( std::move( m1234_Z), "m1234Z"); 
    iEvent.put( std::move( m12_top), "m12top"); 
    iEvent.put( std::move( m23_top), "m23top"); 
    iEvent.put( std::move( m13_top), "m13top"); 
    iEvent.put( std::move( m1234_top), "m1234top"); 
    iEvent.put( std::move( genPt), "genPt");
    iEvent.put( std::move( dRjetParticle), "dRjetParticle");
    iEvent.put( std::move( topSize), "topSize");

    return;
}


float BESTProducer::LegP(float x, int order){
    /* Calculate LegP */
    if (order == 0) return 1;
    else if (order == 1) return x;
    else if (order == 2) return 0.5*(3*x*x - 1);
    else if (order == 3) return 0.5*(5*x*x*x - 3*x);
    else if (order == 4) return 0.125*(35*x*x*x*x - 30*x*x + 3);
    else return 0;
}


int BESTProducer::FWMoments( std::vector<TLorentzVector> particles, double (&outputs)[5] ){
    /* Fox-Wolfram Moments */
    unsigned int numParticles = particles.size();

//    float s(0.0);
    float H0(0.0);
    float H4(0.0);
    float H3(0.0);
    float H2(0.0);
    float H1(0.0);

    for (unsigned int i=0; i<numParticles; i++){
//        s += particles[i].E();
        for (unsigned int j=i; j<numParticles; j++){
            float costh = ( particles[i].Px() * particles[j].Px() + particles[i].Py() * particles[j].Py() + particles[i].Pz() * particles[j].Pz() ) / ( particles[i].P() * particles[j].P() );
            float w1 = particles[i].P();
            float w2 = particles[j].P();

            float fw0 = LegP(costh, 0);
            float fw1 = LegP(costh, 1);
            float fw2 = LegP(costh, 2);
            float fw3 = LegP(costh, 3);
            float fw4 = LegP(costh, 4);

            H0 += w1 * w2 * fw0;
            H1 += w1 * w2 * fw1;
            H2 += w1 * w2 * fw2;
            H3 += w1 * w2 * fw3;
            H4 += w1 * w2 * fw4;
        } // end second loop over particles
    } // end first loop over particles

    H0 += 0.001;
    outputs[0] = (H0);
    outputs[1] = (H1 / H0);
    outputs[2] = (H2 / H0);
    outputs[3] = (H3 / H0);
    outputs[4] = (H4 / H0);

    return 0;
}


void BESTProducer::pboost( TVector3 pbeam, TVector3 plab, TLorentzVector &pboo ){
    /* given jet constituent momentum plab, find momentum relative to beam direction pbeam */
    double pl = plab.Dot(pbeam);
    pl *= 1 / pbeam.Mag();
    // double pt = sqrt(plab.Mag()*plab.Mag()-pl*pl);

    TVector3 pbx;  // set x axis direction along pbeam x (0,0,1)
    TVector3 pby;  // set y axis direction along -pbx x pbeam

    pbx.SetX(pbeam.Y());
    pbx.SetY(pbeam.X());
    pbx.SetZ(0.0);
    pbx *= 1./pbx.Mag();

    pby  = -pbx.Cross(pbeam);
    pby *= 1./pby.Mag();

    pboo.SetX((plab.Dot(pbx)));
    pboo.SetY((plab.Dot(pby)));
    pboo.SetZ(pl);

    return;
}


// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
BESTProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(BESTProducer);
