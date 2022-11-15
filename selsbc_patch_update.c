// *********************************************************************
// Select PS Candidates
// Input are SLC's
// ---------------------------------------------------------------------
// AUTHOR    : Andy Hooper
// ---------------------------------------------------------------------
// WRITTEN   : 04.08.2003
//  
// Change History
// ==============================================
// 03/2009 MA  Fix for gcc 4.3.x
// 12/2012 LIG Speed up processing by reading blocks
// 12/2012 DB  Merge with developers version (SVN)
// 02/2018 DB  Read 10 lines at the time to handle larger temporal datasets
// ==============================================

#include <iostream>  
using namespace std;
     
#include <fstream>  
using namespace std;

#include <string.h>  
using namespace std;
     
#include <complex>  
using namespace std;
     
#include <vector>  
using namespace std;
     
#include <cmath>  
using namespace std;
     
#include <cstdio>
using namespace std;     

#include <cstdlib>     
using namespace std;     

#include <climits>     
using namespace std;     

// =======================================================================
// Start of program 
// =======================================================================
int cshortswap( complex<short>* f )
{
  char* b = reinterpret_cast<char*>(f);
  complex<short> f2;
  char* b2 = reinterpret_cast<char*>(&f2);
  b2[0] = b[1];
  b2[1] = b[0];
  b2[2] = b[3];
  b2[3] = b[2];
  f[0]=f2;
}

int cfloatswap( complex<float>* f )
{
  char* b = reinterpret_cast<char*>(f);
  complex<float> f2;
  char* b2 = reinterpret_cast<char*>(&f2);
  b2[0] = b[3];
  b2[1] = b[2];
  b2[2] = b[1];
  b2[3] = b[0];
  b2[4] = b[7];
  b2[5] = b[6];
  b2[6] = b[5];
  b2[7] = b[4];
  f[0]=f2;
}
int longswap( int32_t* f )
{
  char* b = reinterpret_cast<char*>(f);
  int32_t f2;
  char* b2 = reinterpret_cast<char*>(&f2);
  b2[0] = b[3];
  b2[1] = b[2];
  b2[2] = b[1];
  b2[3] = b[0];
  f[0]=f2;
}

//int main(long  argc, char *argv[] ) {    
int main(int  argc, char *argv[] ) {   // [MA]  long --> int for gcc 4.3.x 

try {
 
  if (argc < 3)
  {	  
     cout << "Usage: selpsc parmfile patch.in pscands.1.ij pscands.1.da mean_amp.flt precision byteswap maskfile " << endl << endl;
     cout << "input parameters:" << endl;
     cout << "  parmfile (input) amplitude dispersion threshold" << endl;
     cout << "                   width of amplitude files (range bins)" << endl;
     cout << "                   SLC file names & calibration constants" << endl;
     cout << "  patch.in (input) location of patch in rg and az" << endl;
     cout << "  pscands.1.ij   (output) PS candidate locations" << endl;
     cout << "  pscands.1.da   (output) PS candidate amplitude dispersion" << endl << endl;
     cout << "  mean_amp.flt (output) mean amplitude of image" << endl << endl;
     cout << "  precision  (input) s or f (default)" << endl;
     cout << "  byteswap   (input) 1 for to swap bytes, 0 otherwise (default)" << endl;
     cout << "  selpsc.in  (input) !!!!!!!!! key !!!!!!!!!!!!" << endl;
     cout << "  maskfile   (input)  mask rows and columns (optional)" << endl;
     throw "";
  }   
     
//  char *ijname;
  const char *ijname;          // [MA] deprication fix
  if (argc < 4) 
     ijname="pscands.1.ij";
  else ijname = argv[3];   

  char jiname[256]; // float format big endian for gamma
  strcpy (jiname,ijname);
  strcat (jiname,".int");

//  char *ampoutname;
//  if (argc < 4) 
//     ampoutname="pscands.1.amp";
//  else ampoutname = argv[3];   
     
//  char *daoutname;
  const char *daoutname;       // [MA] deprication fix
  if (argc < 5) 
     daoutname="pscands.1.da";
  else daoutname = argv[4];   
     
//  char *meanoutname;
  const char *meanoutname;    // [MA] deprication fix
  if (argc < 6) 
     meanoutname="mean_amp.flt";
  else meanoutname = argv[5];   

  const char *prec;
  if (argc < 7)
     prec="f";
  else prec = argv[6];

  int byteswap;
  if (argc < 8)
     byteswap=0;
  else byteswap = atoi(argv[7]);

  //  char *maskfilename;
  const char *maskfilename;   // [MA] deprication fix
  if (argc < 10) 
     maskfilename="";
  else maskfilename = argv[9];   
     
     
  ifstream parmfile (argv[1], ios::in);
  if (! parmfile.is_open()) 
  {	  
      cout << "Error opening file " << argv[1] << "\n"; 
      throw "";
  }

  ifstream ps_parmfile(argv[8], ios::in);
  if (!ps_parmfile.is_open())
  {
    cout << "Error opening file " << argv[1] << "\n";
    throw "";
  }

  char line[256];
  char line0[256];
  int num_files = 0;
  int num_rslc = 0;

  int width = 0;
  float D_thresh = 0;
  int pick_higher = 0;

  parmfile >> D_thresh;
  cout << "dispersion threshold = " << D_thresh << "\n";
  float D_thresh_sq = D_thresh*D_thresh;
  if (D_thresh<0) { 
      pick_higher=1;
  }

  float tmp = 0;
  ps_parmfile >> tmp;
  int tmp_width = 0;
  ps_parmfile >> tmp_width;

  parmfile >> width;
  cout << "width = " << width << "\n";	  
  parmfile.getline(line,256);
  int savepos=parmfile.tellg();
  parmfile.getline(line,256);
  while (! parmfile.eof())
  {
      parmfile.getline(line,256);
      num_files++;
  }    
  parmfile.clear();
  parmfile.seekg(savepos);
  char ampfilename[256];
  ifstream* ampfile   = new ifstream[num_files];
  float* calib_factor = new float[num_files];

  //add by C. Song --------------------//
  ps_parmfile.getline(line0, 256);
  int savepos0 = ps_parmfile.tellg();
  ps_parmfile.getline(line0, 256);
  while (!ps_parmfile.eof())
  {
    ps_parmfile.getline(line0, 256);
    num_rslc++;
  }
  ps_parmfile.clear();
  ps_parmfile.seekg(savepos0);
  char ampfilename0[256];
  ifstream *ampfile0 = new ifstream[num_rslc];
  float *calib_factor0 = new float[num_rslc];
  //add by C. Song --------------------//
  string rslc_sb[num_files];
  int rslc_sb_index[num_files];
  string rslc_ps[num_rslc];

  /* Comment by C. Song
  for (int i=0; i<num_files; ++i) 
  {
    parmfile >> ampfilename >> calib_factor[i];
    ampfile[i].open (ampfilename, ios::in|ios::binary);
    cout << "opening " << ampfilename << "...\n";

    if (! ampfile[i].is_open())
    {	    
        cout << "Error opening file " << ampfilename << "\n"; 
	      throw "";
    }

    char header[32];
    long magic=0x59a66a95;
    ampfile[i].read(header,32);
    if (*reinterpret_cast<long*>(header) == magic)
        cout << "sun raster file - skipping header\n";
    else ampfile[i].seekg(ios::beg); 
  }
  Comment by C. Song */

  //add by C. Song --------------------//
  for (int i=0; i<num_rslc; ++i) 
  {
    ps_parmfile >> ampfilename0 >> calib_factor0[i];
    string str = ampfilename0;
    str.erase(0, str.length() - 13);
    rslc_ps[i] = str;
    //cout << rslc_ps[i] << "\n";
    ampfile0[i].open (ampfilename0, ios::in|ios::binary);
    cout << "opening " << ampfilename0 << "...\n";

    if (! ampfile0[i].is_open())
    {	    
        cout << "Error opening file " << ampfilename0 << "\n"; 
	      throw "";
    }

    char header0[32];
    long magic0=0x59a66a95;
    ampfile0[i].read(header0,32);
    if (*reinterpret_cast<long*>(header0) == magic0)
        cout << "sun raster file - skipping header\n";
    else ampfile0[i].seekg(ios::beg); 
  }

  for (int i = 0; i < num_files; ++i)
  {
    parmfile >> ampfilename >> calib_factor[i];
    string str = ampfilename;
    str.erase(0, str.length() - 13);
    for (int j = 0; j < num_rslc; ++j)
    {
      if (str == rslc_ps[j])
      {
        rslc_sb_index[i] = j;
        break;
      }
    }
    //cout << rslc_sb_index[i] << "\n";
  }
  //add by C. Song --------------------//


  parmfile.close();
  ps_parmfile.close();
  cout << "number of amplitude files = " << num_files << "\n";
  cout << "number of rslc files = " << num_rslc << "\n";

  ifstream patchfile (argv[2], ios::in);
  if (! patchfile.is_open()) 
  {	  
      cout << "Error opening file " << argv[2] << "\n"; 
      throw "";
  }    

  int rg_start=0;
  int rg_end=INT_MAX;
  int az_start=0;
  int az_end=INT_MAX;
  patchfile >> rg_start;
  patchfile >> rg_end;
  patchfile >> az_start;
  patchfile >> az_end;
  patchfile.close();

  const int sizeoffloat=4; 
  int sizeofelement; 
  if (prec[0]=='s')
  {
      sizeofelement = sizeof(short);
  }else sizeofelement = sizeof(float);

  const int linebytes = width*sizeofelement*2;  // bytes per line in amplitude files (SLCs)


  filebuf *pbuf;
  long size;
  long numlines;

  // get pointer to associated buffer object
  pbuf=ampfile0[0].rdbuf();

  // get file size using buffer's members
  size=pbuf->pubseekoff (0,ios::end,ios::in);
  pbuf->pubseekpos (0,ios::in);
  numlines=size/width/sizeof(float)/2;

  cout << "number of lines per file = " << numlines << "\n";
  
  ifstream maskfile (maskfilename, ios::in);
  char mask_exists = 0;
  if (maskfile.is_open()) 
  {	 
      cout << "opening " << maskfilename << "...\n";
      mask_exists=1;
  }    
  
  ofstream ijfile(ijname,ios::out);
  ofstream jifile(jiname,ios::out);
  ofstream daoutfile(daoutname,ios::out);
  ofstream meanoutfile(meanoutname,ios::out);
 
  int LineInBuffer = 10;
  char* buffer = new char[num_rslc*linebytes*LineInBuffer]; // used to store 100 lines of all amp files
  complex<float>* bufferf = reinterpret_cast<complex<float>*>(buffer);
  complex<short>* buffers = reinterpret_cast<complex<short>*>(buffer);


  char* maskline = new char[width*LineInBuffer];
  for (int x=0; x<width*LineInBuffer; x++) // for each pixel in range
  {
      maskline[x] = 0;
  }

  //int y=0;                                    // amplitude files line number
  int y=az_start-1;                             // Changed by LI Gang to calc the start position of azimuth for each ampfile
  int pscid=0;                                  // PS candidate ID number

  long long pix_start; 				// start position in azimuth in values
  long long pos_start; 				// start position of azimuth in bytes
  pix_start= (long long)(y)*(long long)(width);
  pos_start = (long long)(y) * (long long)(linebytes);
  //end
  
    
  for ( int i=0; i<num_rslc; i++)              // read in first line from each amp file
  {
      ampfile0[i].seekg (pos_start, ios::beg);
      ampfile0[i].read (reinterpret_cast<char*>(&buffer[i*linebytes*LineInBuffer]), linebytes*LineInBuffer);
  } 
     
  if (mask_exists==1) 
  {
     maskfile.seekg (pix_start, ios::beg);      // set pointer to start of patch in mask file
  }

  while (! ampfile0[1].eof() && y < az_end) 
  {
     if (mask_exists==1) 
     {
         maskfile.read (maskline, width*LineInBuffer);
     }    
     if (y >= az_start-1)
     {
      for(int j=0; j<LineInBuffer; j++) //for each line in azimuth within buffer
      {
       for (int x=rg_start-1; x<rg_end; x++) // for each pixel in range
       {
     
        float sumamp = 0;
        float sumampdiffsq = 0;
        int i;

        for (i=0; i<num_files/2; i++)        // for each amp file
	      {
           complex<float> camp1, camp2; // get amp value
           if (prec[0]=='s')
           {
           }
           else
           {
               // modified by C. Song -------------------------//
               int master_index = rslc_sb_index[i*2];
               int slave_index = rslc_sb_index[i*2+1];
               //cout << "master slave: " << master_index << " " << slave_index << "\n";
               camp1 = bufferf[master_index * width * LineInBuffer + j * width + x]; // get amp value
               camp2 = bufferf[slave_index * width * LineInBuffer + j * width + x];  // get amp value
               // modified by C. Song -------------------------//
               if (byteswap == 1)
               {
                  cfloatswap(&camp1);
                  cfloatswap(&camp2);
               }
           }
           float amp1=abs(camp1)/calib_factor[i*2]; // get amp value
           float amp2=abs(camp2)/calib_factor[i*2+1]; // get amp value
           sumamp+=amp1;
           sumamp+=amp2;
           sumampdiffsq+=(amp1-amp2)*(amp1-amp2);
        }
        meanoutfile.write(reinterpret_cast<char*>(&sumamp),sizeof(float));	

        if (maskline[j*width+x]==0 && sumamp > 0)
        { 
	          float D_a=sqrt(sumampdiffsq/(num_files/2))/(sumamp/num_files); 
            if (pick_higher==0 && D_a<D_thresh ||                 \
                pick_higher==1 && D_a>=D_thresh) 
	          {
               ++pscid;

               ijfile << pscid << " " << y << " " << x << "\n"; 

               int32_t J=x;
               int32_t I=y;
               longswap(&J);
               longswap(&I);
               jifile.write(reinterpret_cast<char*>(&J), sizeof(int32_t));
               jifile.write(reinterpret_cast<char*>(&I), sizeof(int32_t));

               daoutfile << D_a << "\n";
            } // endif
        } // endif
       } // x++
       y++;
      }//j++
     } // endif y>az_start-1
      
     LineInBuffer = (az_end-y)>10 ? 10 : az_end - y; //get buffer length for next

     for ( int i=0; i<num_rslc; i++)           // read in next line from each amp file
     {
        ampfile0[i].read (reinterpret_cast<char*>(&buffer[i*linebytes*LineInBuffer]), linebytes*LineInBuffer);
     } 

     //y++;
     
    cout << y-az_start+1 << " lines processed. Buffer contains " << LineInBuffer << " lines\n";
  }  //while
  for (int i=0; i<num_rslc; ++i) 
  {
    ampfile0[i].close();
  }
  ijfile.close();
  //ampoutfile.close();
  daoutfile.close();
  meanoutfile.close();
  if (mask_exists==1) 
  {	  
      maskfile.close();
  }    
  }
  catch( char * str ) {
     cout << str << "\n";
     return(999);
  }   
  catch( ... ) {
    return(999);
  }

  return(0);
       
};

