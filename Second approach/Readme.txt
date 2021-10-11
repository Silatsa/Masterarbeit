This program was implemented with arduino-1.8.15-windows under Windows 10 and should run on the same or a higher version.


The selfPurging_Sender is the code for the redundant components.

The Receiver_Code is the code for the voter.

---------------------------------------------------------------------
                     How to upload the code into ESP32
--------------------------------------------------------------------


First check whether the correct port (e.g. COM5) has been selected in the tool register.
 Then the ID and the value must be set for each board via the variables "boardId" and "boardValue".