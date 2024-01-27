'''

Creates L1ExtraNtuples (L1 Style) using a UCT->GT jump

Authors: L. Dodd, N. Woods, T. Perry, A. Levine, S. Dasu, M. Cepeda, E. Friis (UW Madison)
         I. Ojalvo, D. Belknap
'''

import FWCore.ParameterSet.Config as cms
import os

from FWCore.ParameterSet.VarParsing import VarParsing
process = cms.Process("ReRunningL1")

#process = cms.Process("PickEvent")
process.source = cms.Source ("PoolSource",
                             fileNames = cms.untracked.vstring(
        #"/store/mc/Phys14DR/TT_Tune4C_13TeV-pythia8-tauola/GEN-SIM-RAW/AVE30BX50_tsg_PHYS14_ST_V1-v1/00000/0059CA9A-0D8F-E411-804F-0025905A60F2.root"
	#"root://cms-xrd-global.cern.ch///store/mc/RunIIFall15DR76/TT_TuneCUETP8M1_13TeV-powheg-pythia8/GEN-SIM-RAW/25nsFlat10to50NzshcalRaw_76X_mcRun2_asymptotic_v12_ext4-v1/00000/0060070D-73C7-E511-8539-0CC47A78A3EE.root"
	#"/store/mc/RunIISpring15DR74/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/GEN-SIM-RAW/AsymptFlat10to50bx25Raw_MCRUN2_74_V9-v1/10000/06A465ED-7401-E511-A75E-00259059649C.root"
        "/store/data/Run2023D/ZeroBias/RAW/v1/000/369/869/00000/ebb4bfa3-c235-4534-95f5-5a83f52de1d2.root"
	#"/store/mc/Fall14DR73/SinglePi0Pt50/GEN-SIM-RAW/PF_NoPU_RAW_GSFromWinter15_tsg_MCRUN2_73_V11-v1/20000/6C1CECC5-93C8-E411-94D3-002590596486.root"
	#"root://cmsxrootd.fnal.gov//store/mc/Fall13dr/DYToEE_Tune4C_13TeV-pythia8/GEN-SIM-RAW/tsg_PU40bx25_POSTLS162_V2-v1/00000/063C87E4-476B-E311-9697-00259081A2C8.root"
                             )   
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(256)
)

# Tested on Monte Carlo, for a test with data edit ahead
#process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')
#process.GlobalTag.globaltag = 'auto:run3_data'

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.autoCond import autoCond
process.GlobalTag.globaltag=autoCond['run3_data']

process.load('SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff')
process.load('CalibCalorimetry.CaloTPG.CaloTPGTranscoder_cfi')

# Load emulation and RECO sequences
process.load("L1Trigger.Layer1PatternGenerator.emulationLayer1_cfi")

# Load sequences
process.load("Configuration.Geometry.GeometryIdeal_cff")

process.p1 = cms.Path(
    process.emulationSequence 
)


# Output definition (This isn't used yet!!! Just a place holder)
#process.output = cms.OutputModule("PoolOutputModule",
#    fileName = cms.untracked.string('out.root'),
#    outputCommands = cms.untracked.vstring(#'drop *',
#          'keep *_*_*_ReRunningL1',
#          'keep *_l1extraParticles*_*_*') 
#)


