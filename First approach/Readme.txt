This program was implemented with arduino-1.8.15-windows under Windows 10 and should run on the same or a higher version.


The VoterElectionAndRedundancy is the code for both the voter and the redundant components.

---------------------------------------------------------------------
                     How to upload the code into ESP32
--------------------------------------------------------------------


First check whether the correct port (e.g. COM5) has been selected in the tool register.
 Then the ID and the value must be set for each board via the variables "boardId" and "boardValue".
 There are three standard values that are set for all boards, the "VoterID", "broadcastAddressVoter" and the "broadcastAddressVoterName".
 The VoterID must be the highest ID number of all boardIDs. BroadcastAddressVoter is the Mac address of the voter board and BroadcastAddressVoterName is the Mac address of the voter board as a string name.