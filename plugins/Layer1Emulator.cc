/*
 * =====================================================================================
        Filename:  Layer1Emulator.cc

       Description: Emulate the Layer 1 TPG processing for the Stage 2
       Layer 1 to Layer 2 July Integration Test

         Author:  Isobel Ojalvo, Austin Belknap
         Company:  UW Madison
 * =====================================================================================
 */

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "CondFormats/L1TObjects/interface/L1CaloHcalScale.h"
#include "CondFormats/DataRecord/interface/L1CaloHcalScaleRcd.h"
#include <fstream>

class Layer1Emulator : public edm::EDProducer {
public:
  Layer1Emulator(const edm::ParameterSet& pset);
  virtual ~Layer1Emulator(){}
  void produce(edm::Event& evt, const edm::EventSetup& es);
  bool findHcal(int ieta, int iphi, 
		edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi> > &hcalTpgs, 
		int &hcalEt, int &hcalFG);

  bool writeLink(char CTP7Name[7], int zside, int ietaIn, int iphiIn, unsigned int gctphiIn,
			       //bool writeLink(char CTP7Name[7], int zside, int ietaIn, int iphiIn,
		 edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi> > &hcalTpgs,
		 edm::Handle<EcalTrigPrimDigiCollection> &ecalTpgs);

  bool findEcal(int ieta, int iphi, 
		edm::Handle<EcalTrigPrimDigiCollection> &ecalTpgs, 
		int &ecalEt, int &ecalFG);

  void makeword(unsigned int &outputWord, 
		unsigned int word0, 
		unsigned int word1, 
		unsigned int word2, 
		unsigned int word3);

  void getGCTphi(const int iPhi, unsigned int &gctPhi);
  
private:
  edm::InputTag hcalDigis_;
  edm::InputTag ecalDigis_;
  int hcalValue_;
  int ecalValue_;
  bool debug_;
};

Layer1Emulator::Layer1Emulator(const edm::ParameterSet& pset) {
  hcalDigis_ = pset.getParameter<edm::InputTag>("hcalDigis");
  ecalDigis_ = pset.getParameter<edm::InputTag>("ecalDigis");
  hcalValue_ = pset.getUntrackedParameter<int>("hcalValue",0);
  ecalValue_ = pset.getUntrackedParameter<int>("ecalValue",0);
  debug_ = pset.exists("debug") ? pset.getParameter<bool>("debug") : false;
  produces<HcalTrigPrimDigiCollection>();
}
void Layer1Emulator::produce(edm::Event& evt, const edm::EventSetup& es) {

  int nCTP7s = 36;
  //int nCTP7s = 4;

  struct ctp7{
    char name[7];
    int zside;
    int iphi;
    unsigned int gctphi;
  }CTP7[nCTP7s];

  //CTP7[0].zside =  1; CTP7[0].iphi = 71; sprintf(CTP7[0].name,"CTP7_0"); 
  //CTP7[1].zside = -1; CTP7[1].iphi = 71; sprintf(CTP7[1].name,"CTP7_1"); 
  //CTP7[2].zside =  1; CTP7[2].iphi = 3; sprintf(CTP7[2].name,"CTP7_2"); 
  //CTP7[3].zside = -1; CTP7[3].iphi = 3; sprintf(CTP7[3].name,"CTP7_3"); 
  
  for(int i = 0 ; i < 18; i++){
    int iP = i*2;
    int iM = i*2 + 1;
    CTP7[iP].zside =  1; CTP7[iP].iphi = i*4 + 1; sprintf(CTP7[iP].name,"P"); 
    CTP7[iM].zside = -1; CTP7[iM].iphi = i*4 + 1; sprintf(CTP7[iM].name,"M"); 
    }

  //calculate gct phi for all
  for(unsigned int iCTP7 = 0; iCTP7 < nCTP7s; iCTP7++){
    getGCTphi(CTP7[iCTP7].iphi,CTP7[iCTP7].gctphi);
    std::cout<<"CTP7 iphi "<< CTP7[iCTP7].iphi<<"CTP7 gctiphi "<<CTP7[iCTP7].gctphi<<std::endl;
  }

  edm::ESHandle<L1CaloHcalScale> hcalScale;
  es.get<L1CaloHcalScaleRcd>().get(hcalScale);
  edm::Handle<HcalTrigPrimDigiCollection> hcalTpgs;
  std::auto_ptr<HcalTrigPrimDigiCollection> output(new HcalTrigPrimDigiCollection);
  edm::Handle<EcalTrigPrimDigiCollection> ecalTpgs;

  std::fstream file;
  std::fstream fileLocations;

  char fileName[20];
  sprintf(fileName,"Events.txt");
  file.open(fileName,std::fstream::in | std::fstream::out | std::fstream::app);

  sprintf(fileName,"EventLocations.txt");
  fileLocations.open(fileName,std::fstream::in | std::fstream::out | std::fstream::app);
  bool foundEvent = false;

  //get ecal and hcal digis
  if(evt.getByLabel(ecalDigis_, ecalTpgs))
    if(evt.getByLabel(hcalDigis_, hcalTpgs)){

      //output not needed since this is for creating txt files
      output->reserve(hcalTpgs->size());

      int tpgEt=0,tpgiEta=0,tpgiPhi=0;
      //Write the event!!
      file<<evt.id().run()<<":"<<evt.luminosityBlock()<<":"<<evt.id().event()<<std::endl;
      std::cout<< evt.id().run() << ":" <<evt.luminosityBlock()<<":"<<evt.id().event()<<std::endl;
      //file<<" tpgEt "<<tpgEt<<" tpgiEta "<<tpgiEta<<" tpgiPhi "<<tpgiPhi<<std::endl;
      
      for(unsigned int iCTP7 = 0; iCTP7 < nCTP7s; iCTP7++){
	//each bram is 4 in iphi and 2 in ieta
	int zside = CTP7[iCTP7].zside;
	int iphi  = CTP7[iCTP7].iphi;
	unsigned int gctphi  = CTP7[iCTP7].gctphi;
	
	for(int ieta = 1 ; ieta < 28; ieta=ieta+2){
	  if(!writeLink(CTP7[iCTP7].name,zside,ieta,iphi,gctphi,hcalTpgs,ecalTpgs))
	    std::cout<<"Error Writing Link"<<std::endl;
	}
      }
    }
  fileLocations.close();
  file.close();
  evt.put(output);
}


bool Layer1Emulator::writeLink(char CTP7Name[7], int zside, int ietaIn, int iphiIn, unsigned int gctphiIn,
			       edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi> > &hcalTpgs,
			       edm::Handle<EcalTrigPrimDigiCollection> &ecalTpgs)
{
  std::fstream fileEcal;
  std::fstream fileHcal;
  char fileNameHcal[40];
  char fileNameEcal[40];
  

  sprintf(fileNameHcal,"calo_slice_phi_%02u_%s_ieta_%02i_HCAL.txt",gctphiIn,CTP7Name,ietaIn);
  sprintf(fileNameEcal,"calo_slice_phi_%02u_%s_ieta_%02i_ECAL.txt",gctphiIn,CTP7Name,ietaIn);

  //std::cout<<"fileNameEcal "<<fileNameEcal<<" fileNameHcal "<<fileNameHcal<<std::endl;
  //sprintf(fileNameHcal,"%s_HCAL_Ieta%i_GCTphi%u_HCAL.txt",CTP7Name,ietaIn,gctphiIn);

  fileEcal.open(fileNameEcal,std::fstream::in | std::fstream::out | std::fstream::app);
  fileHcal.open(fileNameHcal,std::fstream::in | std::fstream::out | std::fstream::app);
  
  int ecalEt[8] = {0}; //, ecalEt2 = 0, ecalEt3 = 0, ecalEt4 = 0, ecalEt5 = 0, ecalEt6 = 0, ecalEt7 = 0, ecalEt8 = 0;
  int hcalEt[8] = {0}; //, hcalEt2 = 0, hcalEt3 = 0, hcalEt4 = 0, hcalEt5 = 0, hcalEt6 = 0, hcalEt7 = 0, hcalEt8 = 0;
  int ecalFG[8] = {0};
  int hcalFG[8] = {0};
  
  unsigned int eWord[4] = {0};

  unsigned int hWord[4] = {0};

  int ieta = ietaIn;
  //unsigned int iphi = iphiIn;

  /* Current Implementation
   * TODO:Check filling
   * 
   *            ieta 0     ieta 1
   * iphi 0   hcalET[0]  hcalET[1]
   * iphi 1   hcalET[2]  hcalET[3]
   * iphi 2   hcalET[4]  hcalET[5]
   * iphi 3   hcalET[6]  hcalET[7]
   */ 
  
  for(int iphi = iphiIn, index = 0; iphi < 4 + iphiIn; iphi++, index+=2 ){
    int iphiFind = iphi;
    if(iphiFind == 73) iphiFind = 1;
    if(iphiFind == 74) iphiFind = 2;

    if(!findHcal(  zside*ieta   , iphiFind, hcalTpgs,  hcalEt[index],    hcalFG[index]))
      std::cout<<"Error!" <<std::endl;
  
    if(!findEcal(  zside*ieta   , iphiFind, ecalTpgs,  ecalEt[index],    ecalFG[index]))
      std::cout<<"Error!"<<std::endl;

    if(!findHcal( zside*(ieta+1), iphiFind, hcalTpgs, hcalEt[index+1], hcalFG[index+4]))
      std::cout<<"Error!" <<std::endl;
    
    if(!findEcal( zside*(ieta+1), iphiFind, ecalTpgs, ecalEt[index+1], ecalFG[index+4]))
      std::cout<<"Error!"<<std::endl;
  }
  
  /* Now to make the words per the protocol
   * 
   * Current implementation is:
   *       word0   hcalET[6]    hcalET[4]    hcalET[2]    hcalET[0] 
   *       word1   hcalET[7]    hcalET[5]    hcalET[3]    hcalET[1]
   *       word2   other
   *       word3   other
   */
  
  makeword( eWord[0], ecalEt[0], ecalEt[2], ecalEt[4], ecalEt[6]);
  makeword( eWord[1], ecalEt[1], ecalEt[3], ecalEt[5], ecalEt[7]);

  makeword( hWord[0], hcalEt[0], hcalEt[2], hcalEt[4], hcalEt[6]);
  makeword( hWord[1], hcalEt[1], hcalEt[3], hcalEt[5], hcalEt[7]);

  //if(ecalEt[0]>0||ecalEt[1]>0||ecalEt[2]>0||ecalEt[0]>3)
  //std::cout << "Non Zero eWord[0] " << eWord[0] << std::endl;
  
  fileEcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << eWord[0] << " ";
  fileEcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << eWord[1] << " ";
  fileEcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << eWord[2] << " ";
  fileEcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << eWord[3] << " ";
  fileEcal <<std::endl;
  
  fileHcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << hWord[0] << " ";
  fileHcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << hWord[1] << " ";
  fileHcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << hWord[2] << " ";
  fileHcal <<"0x" << std::hex << std::setfill('0') << std::setw(8) << hWord[3] << " ";
  fileHcal <<std::endl;


  fileHcal.close();
  fileEcal.close();
  
  return true;

}

/*
 * Make a 32bit word
 *            word  3  2  1  0
 * outputWord = 0x FF FF FF FF
 */
void Layer1Emulator::makeword(unsigned int &outputWord, unsigned int word0, unsigned int word1, unsigned int word2, unsigned int word3){
  
  outputWord  = (0xFF & word0)    ;
  outputWord |= (0xFF & word1)<<8 ;
  outputWord |= (0xFF & word2)<<16;
  outputWord |= (0xFF & word3)<<24;

}

bool Layer1Emulator::findHcal(int ieta, int iphi, 
			      edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi> > &hcalTpgs, 
			      int &hcalEt, int &hcalFG)
{
  bool foundDigi = false;  

  for (size_t i = 0; i < hcalTpgs->size(); ++i) {
    HcalTriggerPrimitiveDigi tpg = (*hcalTpgs)[i];
    short ietaRef = tpg.id().ieta();
    short iphiRef = tpg.id().iphi();
    short zside = tpg.id().zside();
    //std::cout<<"HCAL ietaRef "<<ietaRef<<" zside "<<zside<<std::endl;
    if(ieta == ietaRef && iphi == iphiRef && zside == ieta/(abs(ieta))){
      hcalEt = tpg.SOI_compressedEt();
      hcalFG = tpg.SOI_fineGrain();
      foundDigi = true;
      break;
    }
  }

  if(!foundDigi){
    hcalEt = 0;
    hcalFG = 0;
    if(debug_)std::cout<<"No HCAL Digi Found"<<std::endl;
  }

  return true;
}

bool Layer1Emulator::findEcal(int ieta, int iphi, 
			      edm::Handle<EcalTrigPrimDigiCollection> &ecalTpgs, 
			      int &ecalEt, int &ecalFG)
{
  bool foundDigi = false;  

  for (size_t i = 0; i < ecalTpgs->size(); ++i) {
    EcalTriggerPrimitiveDigi tpg = (*ecalTpgs)[i];
    short ietaRef = tpg.id().ieta();
    short iphiRef = tpg.id().iphi();
    short zside = tpg.id().zside();

    if(ieta == ietaRef && iphi == iphiRef && zside == ieta/(abs(ieta))){
      //if(tpg.compressedEt()>0)
	//std::cout<<"ECAL ietaRef "<<ietaRef<<" iphiRef "<< iphiRef<<" zside "<<zside<< " tpgET "<< tpg.compressedEt() <<std::endl;
      ecalEt = tpg.compressedEt();
      ecalFG = tpg.fineGrain();
      foundDigi = true;
      break;
    }
  }

  if(!foundDigi){
    ecalEt = 0;
    ecalFG = 0;
    if(debug_)std::cout<<"No ECAL Digi Found"<<std::endl;
  }

  return true;
}



void Layer1Emulator::getGCTphi(const int iPhi, unsigned int &gctPhi){
  if(iPhi>=1&&iPhi<=2)
    gctPhi = 0;
  else if(iPhi>=71&&iPhi<=72)
    gctPhi = 0;
  else
    gctPhi = (((unsigned int) iPhi)+1)/4;

  //std::cout<<"iPhi "<<iPhi<<" gctPhi "<< gctPhi<<std::endl;
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(Layer1Emulator);
