/*
 * =====================================================================================
        Filename:  HcalTpgMipEmbedder.cc

       Description: Modify the HCAL TPGs according to the proposed HTR
       modification.  If the HCAL is above a given energy threshold, set the MIP
       bit.

         Author:  Evan Friis, evan.friis@cern.ch
         Company:  UW Madison
 * =====================================================================================
 */

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
//#include "CondFormats/L1TObjects/interface/L1CaloHcalScale.h"
//#include "CondFormats/DataRecord/interface/L1CaloHcalScaleRcd.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGTranscoder.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGRecord.h"

#include <iostream>

using namespace std;

class HcalTpgMipEmbedder : public edm::stream::EDProducer<> {
  public:
    HcalTpgMipEmbedder(const edm::ParameterSet& pset);
    virtual ~HcalTpgMipEmbedder(){}
    void produce(edm::Event& evt, const edm::EventSetup& es);
  private:
    double threshold_;
    int rawthreshold_;
    bool cutOnRawBits_;
    edm::InputTag src_;
    bool debug_;
    edm::ESGetToken<CaloTPGTranscoder, CaloTPGRecord> decoderToken_;
    //edm::ESGetToken<L1CaloHcalScale, L1CaloHcalScaleRcd> hcalScaleToken_;
};

HcalTpgMipEmbedder::HcalTpgMipEmbedder(const edm::ParameterSet& pset) {
  src_ = pset.getParameter<edm::InputTag>("src");
  threshold_ = pset.getParameter<double>("threshold");
  rawthreshold_ = pset.getParameter<unsigned int>("rawThreshold");
  cutOnRawBits_ = pset.getParameter<bool>("cutOnRawBits");
  debug_ = pset.exists("debug") ? pset.getParameter<bool>("debug") : false;
  produces<HcalTrigPrimDigiCollection>();
  decoderToken_ = esConsumes<CaloTPGTranscoder, CaloTPGRecord>();
  //hcalScaleToken_ = esConsumes<L1CaloHcalScale, L1CaloHcalScaleRcd>();
}
void HcalTpgMipEmbedder::produce(edm::Event& evt, const edm::EventSetup& es) {

  //edm::ESHandle<L1CaloHcalScale> hcalScale;
  //es.get<L1CaloHcalScaleRcd>().get(hcalScale);
  //hcalScale = es.getHandle(hcalScaleToken_);
  edm::ESHandle<CaloTPGTranscoder> decoder;
  decoder = es.getHandle(decoderToken_);

  //if (debug_) {
  //  std::cout << "==== event ====" << std::endl;
  //  hcalScale->print(std::cout);
  //}

  edm::Handle<HcalTrigPrimDigiCollection> tpgs;
  evt.getByLabel(src_, tpgs);

  std::unique_ptr<HcalTrigPrimDigiCollection> output(make_unique<HcalTrigPrimDigiCollection>());
  //output->reserve(tpgs->size());

  for (size_t i = 0; i < tpgs->size(); ++i) {
    HcalTriggerPrimitiveDigi tpg = (*tpgs)[i];
    short ieta = std::abs(tpg.id().ieta());
    short zside = tpg.id().zside();
    // Get the real energy
    //double energy = hcalScale->et(tpg.SOI_compressedEt(), ieta, zside);
    double energy = decoder->hcaletValue(tpg.id(), tpg.t0());
    if (debug_) {
      std::cout << ieta << " " << energy << std::endl;
    }

    bool setMIP = false;
    if (cutOnRawBits_) {
      if (tpg.SOI_compressedEt() > rawthreshold_)
        setMIP = true;
    } else {
      if (energy > threshold_) {
        setMIP = true;
      }
    }
    if (setMIP) {
      //std::cout << "Setting mip for: et: " << tpg.SOI_compressedEt() << " eta: " << ieta << " z: " << zside <<  " real: " << energy << std::endl;
      // Set the MIP bit
      HcalTriggerPrimitiveSample new_t0(tpg.t0().raw() | 0x100);
      tpg.setSample(tpg.presamples(), new_t0);
    }
    output->push_back(tpg);
  }

  evt.put(std::move(output),"HcalTpgMipEmbedder");
  //evt.put(output);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(HcalTpgMipEmbedder);
