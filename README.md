Stage-2 Layer 1 Pattern Generator
========================

Installation:

cmsrel CMSSW_7_6_3
cd CMSSW_7_6_3/src
git cms-init
git cms-addpkg L1Trigger
cd L1Trigger/
git clone git@github.com:isobelojalvo/Layer1PatternGenerator.git
cd ../
scram b -j 9 

Usage:
In Layer1PatternGenerator/test/
cmsRun Layer1Generation.py

This will emulate the response of Layer1 for a given MC sample
