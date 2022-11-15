# UpdateStaMPS
This is an update of mt_prep_gamma in StamMPS, which will not be limited by memory in the SBAS processing.
Note that default StaMPS can only read a limited number of interferograms (e.g., 510).

Steps to use:
1) compile selsbc_patch_update.c and pscphase_update.c in Linux/Mac: 
   >> g++ selsbc_patch_update.c -o selsbc_patch_update
   
   >> g++ pscphase_update.c -o pscphase_update
2) export PATH of these scripts or save them to the 'bin' directory of StaMPS.
3) prepare the file selpsc.in, which is used to read amplitude data to save system memory.
   this file can be automatically generate through PS processing in StaMPS with the format as follows.
   
   <img width="573" alt="image" src="https://user-images.githubusercontent.com/114601224/201827065-77c5da5c-8e58-4b07-8167-55626e2ca744.png">
   where, the first colomun is the location/name of RSLC files.
4) run mt_prep_gamma_update.
