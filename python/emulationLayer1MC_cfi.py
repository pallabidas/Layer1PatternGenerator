
from L1Trigger.Layer1PatternGenerator.emulationLayer1_cfi import *  # NOQA
from L1Trigger.Configuration.ValL1Emulator_cff import *  # NOQA

print ("Using workarounds for MC RAW data content bugs")

# We use valHcalTriggerPrimitiveDigis since we emulate these from the full
# hcalDigis readout.  Otherwise it depends on the presence of the sim digis.
uctDigiStep += valHcalTriggerPrimitiveDigis
hackHCALMIPs.src = cms.InputTag("valHcalTriggerPrimitiveDigis")
# Otherwise everything is zero.
HcalTPGCoderULUT.LUTGenerationMode = True






