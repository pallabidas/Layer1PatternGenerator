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
        "/store/mc/Phys14DR/TT_Tune4C_13TeV-pythia8-tauola/GEN-SIM-RAW/AVE30BX50_tsg_PHYS14_ST_V1-v1/00000/0059CA9A-0D8F-E411-804F-0025905A60F2.root"
                             )   
                             )

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(256)
)

# Tested on Monte Carlo, for a test with data edit ahead
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = 'POSTLS161_V12::All'

# Load emulation and RECO sequences
process.load("L1Trigger.Layer-1-PatternGenerator.emulationLayer1MC_cfi")

# Load sequences
process.load("Configuration.Geometry.GeometryIdeal_cff")

process.p1 = cms.Path(
    process.emulationSequence 
)


# Output definition (This isn't used yet!!! Just a place holder)
process.output = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('out.root'),
    outputCommands = cms.untracked.vstring(#'drop *',
          'keep *_*_*_ReRunningL1',
          'keep *_l1extraParticles*_*_*') 
)


