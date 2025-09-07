#include "ccmth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <math.h>
#include <thread>
#include <future>
#include <chrono>

using namespace std;

// Create a struct to pass to the threads
struct CCM_asyncData{

    int iEmbeddingDimension,
        iLagTimeStep,
        iLibraryLength,
        iMaxAutoCorrToCheck,
        iNumberOfTimeStep2Check,
        iEstimatedYLength,
	iYEmbeddingDimension,
	iYLagTimeStep;

    double dCompareTolerance,
           dWeightToleranceLevel,
	   dCCMcorr;

    bool bL2;

    double *dX,*dY,*dEstimatedY;

};

// Simple functions to help stay organized
void CmdLineHelp();
bool ReadCmdLineArgs(int nrhs,const bxArray *prhs[]);
double CCMcorr_async(CCM_asyncData);

// Globals read from command line
int iEmbeddingDimension = -1,
    iLagTimeStep = -1,
    iLibraryLength = -1,
    iNumOfTimeSeries = -1,
    iNumThreads = 0,
    iThreadWaitSec = -1,
    iMaxAutoCorrToCheck = -1,
    iNumberOfTimeStep2Check = -1,
    iYEmbeddingDimension = -1,
    iYLagTimeStep = -1;

double dCompareTolerance = -1,
       dWeightToleranceLevel = -1;

std::string cOutputname,
     cTSOutputname,
     cFilename; //this data file is assumed to be in the proper format (see "usage")

bool bVerboseFlag = false,
     bOptThreading = false,
     bSaveEstimatedTS = false,
     bFindLag = false,
     bFindE = false,
     bPAI = false,
     bPAICalcPart1 = false,
     bPAICalcPart2 = false,
     bL2 = false;

//ccm 
void cmd_ccm(int nlhs, bxArray *plhs[], int nrhs, const bxArray *prhs[]){

    bool bCmdLineOK = ReadCmdLineArgs(nrhs,prhs);
    if( !bCmdLineOK ){
        return;
    }

    char cBuffer[1024]; //read in buffer
    char *cColValue, //used for read in
         *cRowValues; //used for read in
    int iter = 0;

    int iEstimatedY_Length = iLibraryLength-((iEmbeddingDimension-1)*iLagTimeStep);
    double dX[iLibraryLength],
           dY[iLibraryLength],
           dEstimatedY[iEstimatedY_Length];

    //Populate estimate time series, just in case
    for(int iPre = 0;iPre < iEstimatedY_Length;iPre++ ){
        dEstimatedY[iPre] = nan("");
    }

    //Declare things needed for threading
    double dCCMcorrs[iNumThreads][4];
    future<double> WorkerThreads[iNumThreads][4];
    future_status WorkerStatus[4];
    CCM_asyncData sThreadData;
    sThreadData.iEmbeddingDimension = iEmbeddingDimension;
    sThreadData.iLagTimeStep = iLagTimeStep;
    sThreadData.iYEmbeddingDimension = iYEmbeddingDimension;
    sThreadData.iYLagTimeStep = iYLagTimeStep;
    sThreadData.iLibraryLength = iLibraryLength;
    sThreadData.iMaxAutoCorrToCheck = iMaxAutoCorrToCheck;
    sThreadData.dCompareTolerance = dCompareTolerance;
    sThreadData.iNumberOfTimeStep2Check = iNumberOfTimeStep2Check;
    sThreadData.dWeightToleranceLevel = dWeightToleranceLevel;
    sThreadData.iEstimatedYLength = iEstimatedY_Length;
    sThreadData.dEstimatedY = dEstimatedY;
    sThreadData.bL2 = bL2;

    if( bVerboseFlag ){
            bxPrintf("Input data file: %s\nOutput file: %s\nNumber of time series in input file: %i\nLibrary length: %i\nEmbedding dimension: %i\nLag time step: %i\n",cFilename.c_str(),cOutputname.c_str(),iNumOfTimeSeries,iLibraryLength,iEmbeddingDimension,iLagTimeStep);
    }

    //open data input file
    FILE *ifstream = fopen(cFilename.c_str(),"r");
    if(ifstream == NULL){
        bxPrintf("Error: Cannot open %s\n",cFilename.c_str());
            return;
    }

    //open [-o] output file
    FILE *ofstream = fopen(cOutputname.c_str(),"w");
    if(ofstream == NULL){
        bxPrintf("Error: Cannot open %s\n",cOutputname.c_str());
        return;
    }

    //open [-eY] output file
    FILE *ofstreamTS = fopen(cTSOutputname.c_str(),"w");
    if(ofstream == NULL){
        bxPrintf("Error: Cannot open %s\n",cTSOutputname.c_str());
        return;
    }

    if( bVerboseFlag ){ bxPrintf("Processing %s...\n",cFilename.c_str()); }
    int iCurrentThreadsUsed = 0,
        iCheckThread = 0;
    bool bFoundOpenThread;
    for(int iTSiter = 0;iTSiter < iNumOfTimeSeries;iTSiter++ ){

        if( bVerboseFlag ){ bxPrintf("TS %i:",iTSiter); }
        if( bVerboseFlag ){ bxPrintf("Reading TS..."); }
        bFoundOpenThread = false;

        //read in dX and dY
        for( iter=0;iter < iLibraryLength;iter++ ){
                cRowValues = fgets(cBuffer,sizeof(cBuffer),ifstream);
                cColValue = strtok(cRowValues,",");
                if( cColValue != NULL ){
                   dX[iter] = atof(cColValue);
                }else{
                   bxPrintf("Error: %s is not formatted correctly\n",cFilename.c_str());
                }

                cColValue = strtok(NULL,";");
                if( cColValue != NULL ){
                   dY[iter] = atof(cColValue);
                }else{
                   bxPrintf("Error: %s is not formatted correctly\n",cFilename.c_str());
                }
        }

        if( bVerboseFlag ){ bxPrintf("Writing CCMs..."); }

        if( iNumThreads == 0 ){

              if( bPAI ){

                  sThreadData.dX = dX;
                  sThreadData.dY = dY;
                  bPAICalcPart1 = true;
                  fprintf(ofstream,"%.20f,",CCMcorr_async(sThreadData));
                  if( bSaveEstimatedTS ){
                      for(int iPrintStep = 0;iPrintStep < sThreadData.iEstimatedYLength;iPrintStep++ ){
			fprintf(ofstreamTS,"%.20f\n",sThreadData.dEstimatedY[iPrintStep]);
                      }
                  }
                  bPAICalcPart1 = false;
                  bPAICalcPart2 = true;
                  fprintf(ofstream,"%.20f,",CCMcorr_async(sThreadData));
                  //bxPrintf("%.20f,",CCMcorr_async(sThreadData));
                  if( bSaveEstimatedTS ){
                      for(int iPrintStep = 0;iPrintStep < sThreadData.iEstimatedYLength;iPrintStep++ ){
			fprintf(ofstreamTS,"%.20f\n",sThreadData.dEstimatedY[iPrintStep]);
                      }
                  }
                  bPAICalcPart2 = false;

                  bPAICalcPart1 = true;
                  sThreadData.dX = dY;
                  sThreadData.dY = dX;
                  fprintf(ofstream,"%.20f,",CCMcorr_async(sThreadData));
                  if( bSaveEstimatedTS ){
                      for(int iPrintStep = 0;iPrintStep < sThreadData.iEstimatedYLength;iPrintStep++ ){
			fprintf(ofstreamTS,"%.20f\n",sThreadData.dEstimatedY[iPrintStep]);
                      }
                  }
                  bPAICalcPart1 = false;
                  bPAICalcPart2 = true;
                  fprintf(ofstream,"%.20f\n",CCMcorr_async(sThreadData));
                  if( bSaveEstimatedTS ){
                      for(int iPrintStep = 0;iPrintStep < sThreadData.iEstimatedYLength;iPrintStep++ ){
			fprintf(ofstreamTS,"%.20f\n",sThreadData.dEstimatedY[iPrintStep]);
                      }
                  }
                  bPAICalcPart2 = false;

              }else{

                  sThreadData.dX = dX;
                  sThreadData.dY = dY;
                  fprintf(ofstream,"%.20f,",CCMcorr_async(sThreadData));
                  if( bSaveEstimatedTS ){
                      for(int iPrintStep = 0;iPrintStep < sThreadData.iEstimatedYLength;iPrintStep++ ){
			fprintf(ofstreamTS,"%.20f\n",sThreadData.dEstimatedY[iPrintStep]);
                      }
                  }

                  sThreadData.dX = dY;
                  sThreadData.dY = dX;
                  fprintf(ofstream,"%.20f\n",CCMcorr_async(sThreadData));
                  if( bSaveEstimatedTS ){
                      for(int iPrintStep = 0;iPrintStep < sThreadData.iEstimatedYLength;iPrintStep++ ){
			fprintf(ofstreamTS,"%.20f\n",sThreadData.dEstimatedY[iPrintStep]);
                      }
                  }

              }

        }else{
            if( iCurrentThreadsUsed < iNumThreads ){

                sThreadData.dX = dX;
                sThreadData.dY = dY;
                WorkerThreads[iCurrentThreadsUsed][0] = async(launch::async,CCMcorr_async,sThreadData);

                sThreadData.dX = dY;
                sThreadData.dY = dX;
                WorkerThreads[iCurrentThreadsUsed][1] = async(launch::async,CCMcorr_async,sThreadData);

                iCurrentThreadsUsed++;

            }else{

                if( bOptThreading ){

                    while( !bFoundOpenThread ){
                        if( WorkerThreads[iCheckThread][0].valid() && WorkerThreads[iCheckThread][1].valid() ){

                            WorkerStatus[0] = WorkerThreads[iCheckThread][0].wait_for(chrono::seconds(iThreadWaitSec));
                            WorkerStatus[1] = WorkerThreads[iCheckThread][1].wait_for(chrono::seconds(iThreadWaitSec));

                            if( (WorkerStatus[0] == future_status::ready) && (WorkerStatus[1] == future_status::ready) ){
				
				fprintf(ofstream,"%.20f,",WorkerThreads[iCheckThread][0].get());
				fprintf(ofstream,"%.20f\n",WorkerThreads[iCheckThread][1].get());

                                sThreadData.dX = dX;
                                sThreadData.dY = dY;
                                WorkerThreads[iCheckThread][0] = async(launch::async,CCMcorr_async,sThreadData);

                                sThreadData.dX = dY;
                                sThreadData.dY = dX;
                                WorkerThreads[iCheckThread][1] = async(launch::async,CCMcorr_async,sThreadData);

                                bFoundOpenThread = true;

                            }

                        }else if(!WorkerThreads[iCheckThread][0].valid()&&!WorkerThreads[iCheckThread][1].valid() ){

                            sThreadData.dX = dX;
                            sThreadData.dY = dY;
                            WorkerThreads[iCheckThread][0] = async(launch::async,CCMcorr_async,sThreadData);

                            sThreadData.dX = dY;
                            sThreadData.dY = dX;
                            WorkerThreads[iCheckThread][1] = async(launch::async,CCMcorr_async,sThreadData);

                            bFoundOpenThread = true;

                        }else{

                            iCheckThread++;
                            if( iCheckThread > iNumThreads ){
                                iCheckThread = 0;
                            }

                        }

                    }

                }else{

                    for(int iThreadIter = 0;iThreadIter < iNumThreads;iThreadIter++ ){

                        if( WorkerThreads[iThreadIter][0].valid() && WorkerThreads[iThreadIter][1].valid() ){
                            dCCMcorrs[iThreadIter][0] = WorkerThreads[iThreadIter][0].get();
                            dCCMcorrs[iThreadIter][1] = WorkerThreads[iThreadIter][1].get();
                        }

                    }

                    iCurrentThreadsUsed = 0;

                    sThreadData.dX = dX;
                    sThreadData.dY = dY;
                    WorkerThreads[iCurrentThreadsUsed][0] = async(launch::async,CCMcorr_async,sThreadData);

                    sThreadData.dX = dY;
                    sThreadData.dY = dX;
                    WorkerThreads[iCurrentThreadsUsed][1] = async(launch::async,CCMcorr_async,sThreadData);

                    for(int iThreadIter = 0;iThreadIter < iNumThreads;iThreadIter++ ){
                        fprintf(ofstream,"%.20f,",dCCMcorrs[iThreadIter][0]);
                        fprintf(ofstream,"%.20f\n",dCCMcorrs[iThreadIter][1]);
                        dCCMcorrs[iThreadIter][0] = nan("");
                        dCCMcorrs[iThreadIter][1] = nan("");
                    }

                    iCurrentThreadsUsed++;
                }
            }
        }

        if( bVerboseFlag ){ bxPrintf(" done.\n"); }

    }

    for(int iThreadIter = 0;iThreadIter < iNumThreads;iThreadIter++ ){

        if( WorkerThreads[iThreadIter][0].valid() && WorkerThreads[iThreadIter][1].valid() ){
            fprintf(ofstream,"%.20f,",WorkerThreads[iThreadIter][0].get());
            fprintf(ofstream,"%.20f\n",WorkerThreads[iThreadIter][1].get());

        }

    }

    fclose(ifstream);
    fclose(ofstream);
    fclose(ofstreamTS);

    if( bVerboseFlag ){ bxPrintf(" Finished.\n"); }

    return;
}

void CmdLineHelp(){

    bxPrintf("usage: ccm <command line flags>\n\n");
    bxPrintf("basic command line flags:\n");
    bxPrintf("  -E [integer]  : embedding dimension (ignored if using -FindLag or -FindE)\n");
    bxPrintf("  -t [integer]  : lag time step (ignored if using -FindLag)\n");
    bxPrintf("  -L [integer]  : library length of the time series\n");
    bxPrintf("                 (assumed to be equal for X and Y)\n");
    bxPrintf("  -f [string]   : filename of a text file containing time series\n");
    bxPrintf("                 data for X and Y with columns seperated by\n");
    bxPrintf("                 commas and rows seperated by semicolons \n");
    bxPrintf("                 (it is assumed that there are only X and Y\n");
    bxPrintf("                 in the file, i.e. only two columns)\n");
    bxPrintf("  -n [integer]  : number of time series in the -f file\n");
    bxPrintf("  -o [string]   : filename of the output text file\n");
    bxPrintf("input data file format:\n");
    bxPrintf("   X0,Y0;\n");
    bxPrintf("   X1,Y1;\n");
    bxPrintf("   X2,Y2;\n");
    bxPrintf("   X3,Y3;\n");
    bxPrintf("   X4,Y4;\n");
    bxPrintf("   ...\n");
    bxPrintf("[-o] output data file format:\n");
    bxPrintf("   CCM(X,Y),CCM(Y,X) [time series 1]\n");
    bxPrintf("   CCM(X,Y),CCM(Y,X) [time series 2]\n");
    bxPrintf("   ...\n");
    bxPrintf("\nMore details can be found at https://github.com/<githubaccount>/...\n");

}

bool ReadCmdLineArgs(int nrhs, const bxArray *prhs[]){

    bool bInputSet = false,
         bOutputSet = false;

    for(int iter=0;iter < nrhs;iter++ ){
        if( !(bxIsChar(prhs[iter])||bxIsString(prhs[iter])) ){
            CmdLineHelp();
            return(false);
        }  
        std::string argv_iter = get_string(prhs[iter]);
        if( (strcmp("-h",argv_iter.c_str()) == 0) || 
          (strcmp("-?",argv_iter.c_str()) == 0) ){
            CmdLineHelp();
            return( false );

        }else if( strcmp("-E",argv_iter.c_str()) == 0 ){
            if(iter+1 >= nrhs){
              bxPrintf("-E的后面缺少参数\n");
              return( false );
            }
            double argv_val; 
            if(get_double_check(argv_val, prhs[iter+1])){
              bxPrintf("-E的后面要接一个double类型的整数,嵌入流形的长度\n");
              return( false );
            }

            iEmbeddingDimension = int(argv_val);
            iter++;

        }else if( strcmp("-t",argv_iter.c_str()) == 0 ){
            if(iter+1 >= nrhs){
              bxPrintf("-t的后面缺少参数\n");
              return( false );
            }
            double argv_val; 
            if(get_double_check(argv_val, prhs[iter+1])){
              bxPrintf("-t的后面要接一个double类型的整数,表示LagTime\n");
              return( false );
            }

            iLagTimeStep = int(argv_val);
            iter++;

        }else if( strcmp("-n",argv_iter.c_str()) == 0 ){
            if(iter+1 >= nrhs){
              bxPrintf("-n的后面缺少参数\n");
              return( false );
            }
            double argv_val; 
            if(get_double_check(argv_val, prhs[iter+1])){
              bxPrintf("-n的后面要接一个double类型的整数\n");
              return( false );
            }
            iNumOfTimeSeries =int(argv_val);
            iter++;

        }else if( strcmp("-L",argv_iter.c_str()) == 0 ){
            if(iter+1 >= nrhs){
              bxPrintf("-L的后面缺少参数\n");
              return( false );
            }
            double argv_val; 
            if(get_double_check(argv_val, prhs[iter+1])){
              bxPrintf("-L的后面要接一个double类型的整数\n");
              return( false );
            }
            iLibraryLength = int(argv_val);
            iter++;
        }else if( strcmp("-f",argv_iter.c_str()) == 0 ){
            cFilename = get_string(prhs[iter+1]);
            iter++;
            bInputSet = true;
        }else if( strcmp("-o",argv_iter.c_str()) == 0 ){
            cOutputname = get_string(prhs[iter+1]);
            iter++;
            bOutputSet = true;
        }else if( strcmp("-v",argv_iter.c_str()) == 0 ){
            bVerboseFlag = true;
        }else if( strcmp("-PAI",argv_iter.c_str()) == 0 ){
            bPAI = true;
        }else if( strcmp("-L2",argv_iter.c_str()) == 0 ){
            bL2 = true;
        }
    }

    iYEmbeddingDimension = iEmbeddingDimension; //默认相等
    iYLagTimeStep = iLagTimeStep; 
    iNumThreads = 0; //默认不使用多线程
    cTSOutputname = cOutputname+".eY";
    bSaveEstimatedTS = true;

    //Make sure the required things were defined
    if( iEmbeddingDimension < 0 && !bFindLag ){
        bxPrintf("Error: embedding dimension is not defined\n\n");
        CmdLineHelp();
        return( false );
    }
    if( iLagTimeStep < 0 && !bFindLag ){
        bxPrintf("Error: lag time step is not defined\n\n");
        CmdLineHelp();
        return( false );
    }
    if( iLibraryLength < 0 ){
        bxPrintf("Error: library length is not defined\n\n");
        CmdLineHelp();
        return( false );
    }
    if( !bInputSet ){
        bxPrintf("Error: input data filename is not defined\n\n");
        CmdLineHelp();
        return( false );
    }
    if( iNumOfTimeSeries < 0 ){
        bxPrintf("Error: number of time series in input data file not defined\n\n");
        CmdLineHelp();
        return( false );
    }
    if( !bOutputSet ){
        bxPrintf("Error: output data filename is not defined\n\n");
        CmdLineHelp();
        return( false );
    }
    if( bOptThreading && (iThreadWaitSec < 0) ){
        bxPrintf("Error: Thread wait time must be defined as an integer > 0\n\n");
        CmdLineHelp();
        return( false );
    }
    if( !bSaveEstimatedTS ){
        bxPrintf("Error: output data filename for estimated time series not defined\n\n");
        CmdLineHelp();
        return( false );
    }if( bFindLag && (iMaxAutoCorrToCheck < 0) ){
        bxPrintf("Error: maximum autocorrelation lag time to check is not defined\n\n");
        CmdLineHelp();
        return( false );
    }if( bFindLag && (dCompareTolerance < 0) ){
        bxPrintf("Error: autocorrelation comparison tolerance is not defined\n\n");
        CmdLineHelp();
        return( false );
    }if( bFindE && (iNumberOfTimeStep2Check < 0) ){
        bxPrintf("Error: number of time steps to check is not defined\n\n");
        CmdLineHelp();
        return( false );
    }if( bFindE && (dWeightToleranceLevel < 0) ){
        bxPrintf("Error: weight tolerance is not defined\n\n");
        CmdLineHelp();
        return( false );
    }if( bFindE && bFindLag ){
        bxPrintf("Error: -FindLag and -FindE cannot be used together\n\n");
        CmdLineHelp();
        return( false );
    }if( bFindE && bPAI ){
        bxPrintf("Error: -FindE and -PAI cannot be used together\n\n");
        CmdLineHelp();
        return( false );
    }if( bFindLag && bPAI ){
        bxPrintf("Error: -FindLag and -PAI cannot be used together\n\n");
        CmdLineHelp();
        return( false );
    }if( (iNumThreads != 0) && bSaveEstimatedTS ){
        bxPrintf("Error: -eY cannot be used with threading\n\n");
        CmdLineHelp();
        return( false );
    }

    //Issue any warnings
    if( iNumThreads < 0 ){
        bxPrintf("Warning: number of threads improperly defined; using default\n");
        iNumThreads = 0;
    }
    if( bVerboseFlag && (iNumThreads != 0) ){
        bxPrintf("Warning: verbose logging is not available with multiple threads; turning off verbose output\n");
        bVerboseFlag = false;
    }


    return( true );
}

double CCMcorr_async(CCM_asyncData sThreadData){

    double dResult;
    int iResult;

    if( bFindLag ){
        iResult = FindLagTimeStep(sThreadData.dX,sThreadData.iLibraryLength,sThreadData.iMaxAutoCorrToCheck,sThreadData.dCompareTolerance,bVerboseFlag);
        return( (double) iResult );
    }else if( bFindE ){
        iResult = FindEmbeddingDimension(sThreadData.dX,sThreadData.iLibraryLength,sThreadData.iNumberOfTimeStep2Check,sThreadData.dWeightToleranceLevel,sThreadData.iLagTimeStep,bVerboseFlag);
        return( (double) iResult );
    }else if( bPAICalcPart1 ){
        CCMcorr(sThreadData.dCCMcorr,sThreadData.dX,sThreadData.iLibraryLength,sThreadData.dX,sThreadData.iLibraryLength,sThreadData.iEmbeddingDimension,sThreadData.iLagTimeStep,sThreadData.dEstimatedY,sThreadData.iEstimatedYLength,sThreadData.bL2);
	dResult = sThreadData.dCCMcorr;
        return( dResult );
    }else if( bPAICalcPart2 ){
        CCMcorr2(sThreadData.dCCMcorr,sThreadData.dX,sThreadData.iLibraryLength,sThreadData.dX,sThreadData.iLibraryLength,sThreadData.dY,sThreadData.iLibraryLength,sThreadData.iEmbeddingDimension,sThreadData.iLagTimeStep,sThreadData.iYEmbeddingDimension,sThreadData.iYLagTimeStep,sThreadData.dEstimatedY,sThreadData.iEstimatedYLength,sThreadData.bL2);
	dResult = sThreadData.dCCMcorr;
        return( dResult );
    }else{
	CCMcorr(sThreadData.dCCMcorr,sThreadData.dX,sThreadData.iLibraryLength,sThreadData.dY,sThreadData.iLibraryLength,sThreadData.iEmbeddingDimension,sThreadData.iLagTimeStep,sThreadData.dEstimatedY,sThreadData.iEstimatedYLength,sThreadData.bL2);
	dResult = sThreadData.dCCMcorr;
        return( dResult );
    }

}
