#include "Rare.h"

const char* rar_ver="0.92";


rareStruct* calcDivRar(int i, Matrix* Mo, DivEsts* div, options* opts,
        vector<vector<vector<uint>>>* abundInRow, vector<vector<vector<uint>>>* occuencesInRow,
        string outF, int repeats, int writeFiles){

    smplVec* cur        = Mo->getSampleVec(i);
    string curS         = Mo->getSampleName(i);
    div->SampleName     = curS;
    std::vector<vector<uint>> cnts;
    vector<rare_map> cntsMap;
    string cntsName;
    string skippedNames;
    bool wrAtAll(writeFiles > 0);

    cur->rarefy(opts->depth, outF, repeats,
            div, cntsMap, cntsName, skippedNames, abundInRow, occuencesInRow,
            writeFiles, false, wrAtAll);

    // put everything in our nice return container
    rareStruct* tmpRS       = new rareStruct();
    tmpRS->div              = div;
    tmpRS->cnts             = cntsMap;
    tmpRS->cntsName         = cntsName;
    tmpRS->skippedNames     = skippedNames;
    tmpRS->i                = i;

    delete cur;
    return tmpRS;
}


rareStruct* calcDivRarVec(int i, vector<string> fileNames, DivEsts* div, options* opts,
        vector<vector<vector<uint>>>* abundInRow, vector<vector <vector<uint>>>* occuencesInRow, string outF,
        int repeats, int writeFiles){

    smplVec* cur = new smplVec(fileNames[i],4);

    std::vector<vector<uint>> cnts;
    vector< rare_map> cntsMap;
    string cntsName;
    string skippedNames;
    bool wrAtAll(writeFiles > 0);
    cur->rarefy(opts->depth, outF, repeats,
            div, cntsMap, cntsName, skippedNames, abundInRow, occuencesInRow,
            writeFiles, false, wrAtAll);

    rareStruct* tmpRS       = new rareStruct();// 	= {*div, retCnts};
    tmpRS->div              = div;
    tmpRS->cnts             = cntsMap;
    tmpRS->cntsName         = cntsName;
    tmpRS->skippedNames     = skippedNames;
    tmpRS->IDs              = cur->getRowNames();
    tmpRS->i                = i;

    delete cur;

    if( remove( fileNames[i].c_str() ) != 0 ){
        cerr << "LowMem: Error deleting file: " << fileNames[i] << std::endl;
    }
    return tmpRS;
}



void helpMsglegacy(){
    string  AvailableModes = "Available run modes:\nnormalize\nsplitMat\nlineExtr\nmergeMat\nsumMat\nrarefaction\nrare_inmat\nmodule\n";
    cerr << AvailableModes << "Provide two arguments\nexiting..\n";
    cerr << "------------------------------\nAuthor: falk.hildebrand@gmail.com\n";
    std::exit(2);
}

void stateVersion(){
    printf("rarefaction tool kit (rtk) %s\n", rar_ver);
}

void helpMsg(){
    stateVersion();
    printf("\n");
    printf("USAGE\n");
    printf("    rtk <mode> -i <input.csv> -o <output> [options] \n");
    printf("\n");
    printf("MODE rarefaction\n");
    printf("\n");
    printf("OPTIONS\n");

    printf("<mode>      For rarefaction: mode can be either swap or memory.\n");
    printf("            Swap mode creates temporary files but uses less memory. \n");
    printf("            The speed of both modes is comparable.\n");
    printf("    -i      path to an .csv file to rarefy\n");
    printf("    -o      path to a output directory\n");
    printf("    -d      Depth to rarefy to. Default is 0.95 times the minimal column sum.\n");
    printf("    -r      Number of times to create diversity measures. Default is 10.\n");
    printf("    -w      Number of rarefied tables to write.\n");
    printf("    -t      Number of threads to use. Default: 1\n");
    printf("    -ns     If set, no temporary files will be used when writing rarefaction tables to disk.\n");
    printf("\n");
    printf("EXAMPLE\n");
    printf("    Rarefy a table to 1000 counts per sample with two threads. Create one table:\n");
    printf("        rtk swap -i table.csv -o outputdir/prefix. -d 1000 -r 10 -w 1 -t 2\n");
    printf("\n");
    printf("    Rarefy with most memory and least amount of IO:\n");
    printf("        rtk memory -i table.csv -o outputdir/prefix. -ns\n");
    printf("\n");
    printf("MODE: Colsums\n");
    printf("Reports the column sums of all columns in form of a sorted and an unsorted file.\n");
    printf("\n");
    printf("EXAMPLE\n");
    printf("    Repot column sums of file 'table.csv'\n");
    printf("        rtk colsums -i table.csv -o prefix\n");

    printf("\n");

    std::exit(2);
}




vector<long> parseDepths(string a){
    std::vector<long> vect;
    std::stringstream ss(a);

    int i;

    while (ss >> i)
    {
        vect.push_back(i);

        if (ss.peek() == ',')
            ss.ignore();
    }


    return vect;
}

options::options(int argc, char** argv) :input(""), output(""), mode(""),
    referenceDir(""), referenceFile(""),
    depth(), repeats(10), write(0), threads(1), writeSwap(true), verbose(false),
    modDB(""), modRedund(5), modEnzCompl(0.5f), modModCompl(0.5f), modWrXtraInfo(false), 
    modCollapse(false), calcCoverage(false),
    xtra("") {


        bool hasErr = false;
        if (argc == 0) { return; }//could be the pseudo mod opt object

        //bool newIDthrs = false; string newIDthStr("");
        mode = argv[1];

        for (int i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "-i"))
                input = argv[++i];
            else if (!strcmp(argv[i], "-o"))
                output = argv[++i];
            ///else if (!strcmp(argv[i], "-m"))
            else if (!strcmp(argv[i], "-d")){
                // split by any given komma
                depth = parseDepths(argv[++i]);
                depthMin = *std::min_element(depth.begin(), depth.end());
            }else if (!strcmp(argv[i], "-r"))
                repeats = atoi(argv[++i]);
            else if (!strcmp(argv[i], "-w"))
                write = atoi(argv[++i]);
            else if (!strcmp(argv[i], "-t"))
                threads = atoi(argv[++i]);
            else if (!strcmp(argv[i], "-ns"))   // no swap
                writeSwap = false;
            else if (!strcmp(argv[i], "-v"))
                verbose = true;
            else if (!strcmp(argv[i], "-h"))
                helpMsg();
            //geneMat specific args
            else if (!strcmp(argv[i], "-map"))
                map = argv[++i];
            else if (!strcmp(argv[i], "-refD"))
                referenceDir = argv[++i];
            else if (!strcmp(argv[i], "-reference"))
                referenceFile = argv[++i];
            //module specific args
            else if (!strcmp(argv[i], "-refMods"))
                modDB = (argv[++i]);
            else if (!strcmp(argv[i], "-redundancy"))
                modRedund = atoi(argv[++i]);
            else if (!strcmp(argv[i], "-enzymeCompl"))
                modEnzCompl = (float)atof(argv[++i]);
            else if (!strcmp(argv[i], "-moduleCompl"))
                modModCompl = (float)atof(argv[++i]);
            else if (!strcmp(argv[i], "-writeExtraModEstimates"))
                modWrXtraInfo = true;
            else if (!strcmp(argv[i], "-collapseDblModules"))
                modCollapse = true;
            else if (!strcmp(argv[i], "-useCoverage"))//for gene catalog, default is counts
                calcCoverage = true;
            else if (!strcmp(argv[i], "-xtra"))
                xtra = (argv[++i]);



        }

        // sanity checks
        // we need input
        if (input == "") {//just set some defaults
            cerr << "Input must be specified\n";
            hasErr = true;
        }
        if (output == "") {//just set some defaults
            cerr << "Output must be specified\n";
            hasErr = true;
        }

        if (hasErr) {
            cerr << "Use \"rtk -h\" to get full help.\n";
            exit(98);
        }
        if (mode == "rarefaction") {
            if (writeSwap) { mode = "swap"; 
            } else { mode = "memory"; }
        }
    }
void options::print_rare_details(){

    stateVersion();
    // print run mode:
    cout << "------------------------------------ "  << std::endl;
    cout << "Run information:" << std::endl;
    cout << "input file:     " << input  << std::endl;
    cout << "output file:    " << output  << std::endl;
    cout << "mode:           " << mode  << std::endl;
    //	if(depth != 0){
    //cout << "depth:          " << depth  << std::endl;
    //}else{
    //	cout << "depth:          0.95 x min. column sum"   << std::endl;
    //}
    cout << "repeats:        " << repeats  << std::endl;
    cout << "writes:         " << write  << std::endl;
    cout << "threads:        " << threads  << std::endl;
    cout << "use swap:       ";
    if(writeSwap == false){
        cout << "false" << std::endl;
    }else{
        cout << "true" << std::endl;
    }

    //cout << "mode:           " << mode  << std::endl;
    cout << std::endl;

}



void rareExtremLowMem(options * opts, string inF, string outF, int writeFiles, string arg4, int repeats, int numThr = 1, bool storeBinary = false){
    // this mode takes the file, reads it in memory
    // prints the columns to their own files
    // then it loads those files again and
    // rarefies each column
    // the measures are then combines again.

    //split mat code
    cout << "Splitting input matrix to disk" << std::endl;
    vector<string> fileNames;
    Matrix* Mo 	= new Matrix(inF, outF, "", fileNames, false, true);
    vector < string > SampleNames 	= Mo->getSampleNames();
    vector < string > rowNames 		= Mo->getRowNames();
    cout << "Done loading matrix" << std::endl;

    // abundance vectors to hold the number of occurences of genes per row
    // this will be used for ICE, ACE and or Chao2 estimation
    vector<vector<vector<uint>>> occuencesInRow(opts->depth.size(), vector<vector<uint>>(opts->repeats, vector<uint>(Mo->rowNum(),0)));
    vector<vector<vector<uint>>> abundInRow(opts->depth.size(), vector<vector<uint>>(opts->repeats, vector<uint>(Mo->rowNum(),0)));

    for(int i = 0; i < opts->depth.size(); i++){
        if (opts->depth[i] < 1.) {
            // rarefy to smallest colSum
            opts->depth[i] = (uint)round(opts->depth[i] * Mo->getMinColSum());
            if (opts->depth[i] == 0.0) {
                cerr << "Minimal sample count is 0. This can not be the rarefaction depth. Please provide a rarefaction depth > 0." << std::endl;
                exit(1);
            }
        } 
    }
    delete Mo;
    //	uint rareDep = uint(rareDep2);

    int NoOfMatrices = writeFiles;
    vector< vector< rare_map > > MaRare (NoOfMatrices);
    std::vector<string> cntsNames;
    vector < vector < string > > tmpMatFiles (NoOfMatrices );
    vector<DivEsts*> divvs(fileNames.size(),NULL);
    vector < job > slots(opts->threads);
    size_t smpls = Mo->smplNum();
    // now start a async in each slot
    uint i          = 0; 
    bool breakpoint(true);
    while (breakpoint) {

        // check for any finished jobs
        for( uint j = 0; j < slots.size(); j++ ){
            if( i >= smpls){
                breakpoint = false;
                // break in case we have more slots than work
                break;
            }

            if(slots[j].inUse == true && slots[j].fut.wait_for(std::chrono::milliseconds(20)) == std::future_status::ready){

                // move the information
                rareStruct* tmpRS;
                tmpRS               = slots[j].fut.get();
                divvs[tmpRS->i]     = tmpRS->div;
                string curS 	    = SampleNames[tmpRS->i];
                cout << curS << std::endl;
                // add the matrices to the container
                if (NoOfMatrices > 0) {
                    if (opts->writeSwap) {
                        binaryStoreSample(tmpMatFiles, tmpRS, rowNames, outF, cntsNames, true);
                    }
                    else {
                        memoryStoreSample(tmpRS, MaRare, cntsNames, true);
                    }
                }

                delete tmpRS;
                // free slot
                slots[j].inUse = false;
            }

            // open new slots
            if( slots[j].inUse == false){

                slots[j].inUse = true;
                // launch an async task
                DivEsts * div   = new DivEsts();
                div->SampleName = SampleNames[i];
                slots[j].fut    = async(std::launch::async, calcDivRarVec, i, fileNames, div, opts, &abundInRow, &occuencesInRow, outF, opts->repeats, opts->write);

                // send user some feedback
                int thirds = 1;
                if(smpls > 4){
                    thirds = (int) floor((smpls-3)/3);
                }

                if(i < 3 || i % thirds == 0  ){
                    cout << "At Sample " << i+1 << " of " << smpls << " Samples" << std::endl;
                    if(i > 3 && i % thirds == 0 ){
                        cout << "..." << std::endl ;
                    }
                }else if( i == 3){
                    cout << "..." << std::endl ;
                }

                i++;

            }

        }


    }

    // wait for the threads to finish up.
    for(uint j = 0; j < slots.size(); j++){
        if(slots[j].inUse == false ){
            // only copy if there is work to be done
            continue;
        }
        slots[j].fut.wait();
        // move the information
        rareStruct* tmpRS;
        tmpRS               = slots[j].fut.get();
        divvs[tmpRS->i]     = tmpRS->div;
        string curS 	    = SampleNames[tmpRS->i];

        // add the matrices to the container
        if (NoOfMatrices > 0) {
            if (opts->writeSwap) {
                binaryStoreSample(tmpMatFiles, tmpRS, rowNames, outF, cntsNames, true);
            }
            else {
                memoryStoreSample(tmpRS, MaRare, cntsNames, true);
            }
        }

        delete tmpRS;
        // free slot
        slots[j].inUse = false;
    }



    // print the div estimates out into a file
    printDivMat(outF , divvs, true, opts);
    for (size_t i = 0; i < divvs.size(); i++){
        delete divvs[i];
    }

    // write rarefaction matrices to disk
    if(NoOfMatrices > 0){
        //vector< string > rowNames = Mo->getRowNames();
        if(storeBinary == true){
            //printRarefactionMatrix(tmpMatFiles, outF, rareDep, cntsNames, rowNames);
        }else{
            //printRarefactionMatrix(MaRare, outF, rareDep, cntsNames, rowNames);
        }
    }

    // compute chao2, ACE, ICE and write to file
    // compute chao2, ACE, ICE and write to file
    vector<mat_fl> chao2;
    vector<mat_fl> ICE;
    vector<mat_fl> ACE;
   // computeChao2(chao2, abundInRow);
    //computeCE(ICE, abundInRow);
    //computeCE(ACE, occuencesInRow);
    //writeGlobalDiv(ICE, ACE, chao2, outF + "_gDiv.tsv");

    cout << "Finished\n";
}




void binaryStoreSample(vector< vector< string> >& tmpMatFiles, rareStruct* tmpRS, vector<string>& rowNames, string outF,  vector<string>& cntsNames, bool reshapeMap){
    // store vectors of rarefied matrix on hard disk for memory reduction
    if(reshapeMap){
        vector < string > rowIDs = tmpRS->IDs;
        vector < uint > nrowIDs(rowIDs.size());
        // convert ids into integer vector, for remapping the values
        for(uint i = 0; i < rowIDs.size(); i++){
            nrowIDs[i] = std::stoi(rowIDs[i]);
        }
        for(uint i = 0; i < tmpRS->cnts.size(); i++){
            // reshape each vector, as some are zero, and we need to rematch values and rows
            // we use the row Ids which we created correctly when splitting the vector from the input file
            rare_map tmpVec;
            for (auto const& x : tmpRS->cnts[i]){
                tmpVec[nrowIDs[x.first]] = x.second;
            }
            string vecLocation = printSimpleMap(tmpVec,	outF + "tmp_" + std::to_string(i) + tmpRS->cntsName + ".binary",	tmpRS->cntsName, rowNames);
            tmpMatFiles[i].push_back(vecLocation);
        }
    }else{
        for(uint i = 0; i < tmpRS->cnts.size(); i++){
            string vecLocation = printSimpleMap(tmpRS->cnts[i],	outF + "tmp_" + std::to_string(i) + tmpRS->cntsName + ".binary",	tmpRS->cntsName, rowNames);
            tmpMatFiles[i].push_back(vecLocation);
        }
    }
    // save sample name
    if(tmpRS->cntsName.size() != 0){
        cntsNames.push_back(tmpRS->cntsName);
    }
}

void memoryStoreSample(rareStruct* tmpRS, vector< vector< rare_map > >& MaRare,  vector<string>& cntsNames, bool reshapeMap){
    if(reshapeMap){
        vector < string > rowIDs = tmpRS->IDs;
        vector < uint > nrowIDs(rowIDs.size());
        // convert ids into integer vector, for remapping the values
        for(uint i = 0; i < rowIDs.size(); i++){
            nrowIDs[i] = std::stoi(rowIDs[i]);
        }
        for(uint i = 0; i < tmpRS->cnts.size(); i++){
            // reshape each vector, as some are zero, and we need to rematch values and rows
            // we use the row Ids which we created correctly when splitting the vector from the input file
            rare_map tmpVec;
            for (auto const& x : tmpRS->cnts[i]){
                tmpVec[nrowIDs[x.first]] = x.second;
            }
            MaRare[i].push_back(tmpVec);
        }

    }else{
        for(uint i = 0; i < tmpRS->cnts.size(); i++){
            MaRare[i].push_back(tmpRS->cnts[i]);
        }
    }
    // save sample name
    if(tmpRS->cntsName.size() != 0){
        cntsNames.push_back(tmpRS->cntsName);
    }
}



void printRarefactionMatrix(vector< vector < string > >& tmpMatFiles, string outF, int rareDep, vector<string>& cntsNames, vector<string>& rowNames){
    //vector < string > rowNames = Mo->getRowNames();
    // reassemble tmp fev files:
    for(uint i = 0; i < tmpMatFiles.size(); i++){
        string matOut = outF + "rarefied_to_" + std::to_string(rareDep) + "_n_" +  std::to_string(i) + ".tsv";
        reassembleTmpMat(tmpMatFiles[i], rowNames, cntsNames, matOut);
        // delete tmp rarefaction files now
        for(uint j = 0; j < tmpMatFiles[i].size(); j++){
            if( remove( tmpMatFiles[i][j].c_str() ) != 0 ){
                cerr << "printRarefactionMatrix: Error deleting file: " << tmpMatFiles[i][j] << std::endl;
            }
        }
    }

}
void printRarefactionMatrix(const vector<vector< rare_map>>& MaRare, string outF, int rareDep, vector<string>& cntsNames, vector<string>& rowNames){
    for(uint i = 0; i < MaRare.size(); i++){
        printRareMat(outF + "rarefied_to_" + std::to_string(rareDep) + "_n_" +  std::to_string(i) + ".tsv", MaRare[i], cntsNames, rowNames);
    }
}





int main(int argc, char* argv[])
{

    if (argc < 2) { cerr << "Not enough arguments. Use \"rtk -h\" for getting started.\n"; exit(3); }

    clock_t tStart = clock();

    options* opts = new options(argc, argv);

    string inF = opts->input;
    string outF = opts->output;
    string mode = opts->mode;
    uint numThr = opts->threads;
    string arg4 = "";//std::to_string(opts->depth[0]);
    string map = opts->map;
    string refD = opts->referenceDir;
    //bool verbose = opts->verbose;



    //all modes that classify as rarefactions:
    if (mode == "rarefaction" || mode == "swap" || mode == "memory") {
        opts->print_rare_details();
    }


    // start the modes
    //	if (argc>3){
    if (mode == "splitMat") {
        vector<string> empt;
        Matrix* Mo = new Matrix(inF, outF, opts->xtra, empt, false);
        //Mo->splitOnHDD(outF);	//Mo->writeSums(outF);
        delete Mo;
        std::exit(0);
        //}else if (mode == "rare_lowMem") {
        //	rareLowMem(inF, outF, writeFiles,  arg4,  repeats, numThr);
}
else if (mode == "correl2") {
    //usage: ./rare correl2 [signature matrix] [output matrix] [big gene matrix]
    //reads in signature matrix (e.g. 40 marker genes)
    //SigMatrix* Sig = new SigMatrix(inF);
    //Sig->estimateBinModel();
    //readMatrixLinebyLine(arg4,Sig);

}
else if (mode == "module") {
    if (argc < 7) {
        cout << "Usage: ./rare module [KO matrix] [outputfile] [module DB file] [KO redundancy, int] [Pathway completeness, float 0-1] [Enzyme completeness, float 0-1]\n";
        cerr << "Not enough arguments for \"module\" function\n";
        exit(3);
    }
    Matrix* Mo = new Matrix(inF, ""); //needs to be KO file
    cerr << "Estimate mod AB\n";
    if (opts->modDB == "") {//try legacy mode
        Mo->estimateModuleAbund(argv, argc);// arg4, outF); //arg4 needs to be module file, outF makes the (several) matrices
    }
    else {
        Mo->estimateModuleAbund(opts);
    }
    delete Mo;
    std::exit(0);
}
else if (mode == "normalize") {
    if (argc < 4) {
        cout << "Usage: ./rare normalize [in matrix] [outputfile]\n";
        cerr << "Not enough arguments for \"normalize\" function\n";
        exit(3);
    }
    Matrix* Mo = new Matrix(inF, ""); //needs to be KO file
    Mo->normalize();
    Mo->writeMatrix(outF);
    delete Mo;
    std::exit(0);
}
else if (mode == "help" || mode == "-help" || mode == "-h" || mode == "--help") {
    helpMsg();
}
else if (mode == "lineExtr") {
    lineCntOut(inF, outF, opts->referenceFile);
    std::exit(0);
}
else if (mode == "mergeMat") {
    VecFiles* VFs = new VecFiles(inF, outF, arg4);
    delete VFs;
    std::exit(0);
}
else if (mode == "sumMat") {
    vector<string> empt;
    Matrix* Mo = new Matrix(inF, outF, refD, empt, true);
    delete Mo;
    std::exit(0);
}
else if (mode == "rarefaction" || mode == "rare_inmat") {
    //rareDep = atoi(arg4.c_str());
    mode = "memory";
}
else if (mode == "colSums" || mode == "colsums" || mode == "colSum") {
    // just load and discard the matrix and report back the colsums
    vector<string> fileNames;
    Matrix* Mo = new Matrix(inF, outF, "", fileNames, false, true, false);
    column co = Mo->getMinColumn();
    vector< pair< double, string>> colsums = Mo->getColSums();
    Mo->writeColSums(outF);

    cout << "" << std::endl;
    cout << "------------------------------------" << std::endl;
    cout << "ColSums output" << std::endl;
    cout << "Smallest column: " << co.id << std::endl;
    cout << "With counts:     " << co.colsum << std::endl;
    cout << "" << std::endl;
    cout << "Colsums where written into the files:" << std::endl;
    cout << "    " << outF << "colSums.txt" << std::endl;
    cout << "    " << outF << "colSums_sorted.txt" << std::endl;

    delete Mo;
    std::exit(0);
}
else if (mode == "geneMat") {
    cout << "Gene clustering matrix creation\n";
    if (argc < 5) { cerr << "Needs at least 4 arguments\n"; std::exit(0); }
    ClStr2Mat* cl = new ClStr2Mat(inF, outF, map, refD,opts->calcCoverage);
    delete cl;
    std::exit(0);
}
else if (mode == "swap") {
    vector < vector < string > > tmpMatFiles(opts->write);
    rareExtremLowMem(opts, inF, outF, opts->write, arg4, opts->repeats, numThr, opts->writeSwap);
    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    std::exit(0);
}
else if (mode == "memory") {
    cout << "Loading input matrix to memory" << std::endl;
    Matrix* Mo = new Matrix(inF, "");//no arg for outfile &  hierachy | gene subset
    vector<DivEsts*> divvs(Mo->smplNum(), NULL);
    vector< string > rowNames = Mo->getRowNames();
    cout << "Done loading matrix" << std::endl;

    // transform all percentage sizes into correct values
    for(int i = 0; i < opts->depth.size(); i++){
        if (opts->depth[i] < 1.) {
            // rarefy to smallest colSum
            opts->depth[i] = (uint)round(opts->depth[i] * Mo->getMinColSum());
            if (opts->depth[i] == 0.0) {
                cerr << "Minimal sample count is 0. This can not be the rarefaction depth. Please provide a rarefaction depth > 0." << std::endl;
                exit(1);
            }
        } 
    }

    // hold rarefied matrices
    // stores : repeats - sampels eg rows - vectors of columns
    int NoOfMatrices = opts->write;
    vector< vector< rare_map > > MaRare(NoOfMatrices);
    std::vector<string> cntsNames;


    // abundance vectors to hold the number of occurences of genes per row
    // this will be used for Chao2 estimation
    vector<vector<vector<uint>>> occuencesInRow(opts->depth.size(), vector<vector<uint>>(opts->repeats, vector<uint>(Mo->rowNum(),0)));
    vector<vector<vector<uint>>> abundInRow(opts->depth.size(), vector<vector<uint>>(opts->repeats, vector<uint>(Mo->rowNum(),0)));

    //object to keep matrices
    vector < vector < string > > tmpMatFiles(opts->write);
    //cerr << "TH";
    // vector keeping all the slots
    vector < job > slots(opts->threads);

    // now start a async in each slot
    uint i          = 0; 


    size_t smpls = Mo->smplNum();
    bool breakpoint(true);
    while (breakpoint) {


        // check for any finished jobs
        for( uint j = 0; j < slots.size(); j++ ){
            if( i >= smpls){
                breakpoint = false;
                // break in case we have more slots than work
                break;
            }


            if(slots[j].inUse == true && slots[j].fut.wait_for(std::chrono::milliseconds(20)) == std::future_status::ready){

                // move the information
                rareStruct* tmpRS;
                tmpRS               = slots[j].fut.get();
                divvs[tmpRS->i]     = tmpRS->div;
                string curS         = Mo->getSampleName(tmpRS->i);

                // add the matrices to the container
                if (NoOfMatrices > 0) {
                    if (opts->writeSwap) {
                        binaryStoreSample(tmpMatFiles, tmpRS, rowNames, outF, cntsNames, false);
                    }
                    else {
                        memoryStoreSample(tmpRS, MaRare, cntsNames, false);
                    }
                }

                delete tmpRS;
                // free slot
                slots[j].inUse = false;
            }

            // open new slots
            if( slots[j].inUse == false){

                slots[j].inUse = true;
                // launch an async task
                DivEsts * div   = new DivEsts();
                slots[j].fut    = async(std::launch::async, calcDivRar, i, Mo, div, opts, &abundInRow, &occuencesInRow, outF, opts->repeats, opts->write);

                // send user some feedback
                int thirds = 1;
                if(smpls > 4){
                    thirds = (int) floor((smpls-3)/3);
                }
                if(i < 3 || i % thirds == 0  ){
                    cout << "At Sample " << i+1 << " of " << smpls << " Samples" << std::endl  ;
                    if(i > 3 && i % thirds == 0 ){
                        cout << "..." << std::endl ;
                    }
                }else if( i == 3){
                    cout << "..." << std::endl ;
                }

                i++;

            }

        }


    }

    // wait for the threads to finish up.
    for(uint j = 0; j < slots.size(); j++){
        if(slots[j].inUse == false ){
            // only copy if there is work to be done
            continue;
        }
        slots[j].fut.wait();
        // move the information
        rareStruct* tmpRS;
        tmpRS               = slots[j].fut.get();
        divvs[tmpRS->i]     = tmpRS->div;
        string curS         = Mo->getSampleName(tmpRS->i);

        // add the matrices to the container
        if (NoOfMatrices > 0) {
            if (opts->writeSwap) {
                binaryStoreSample(tmpMatFiles, tmpRS, rowNames, outF, cntsNames, false);
            }
            else {
                memoryStoreSample(tmpRS, MaRare, cntsNames, false);
            }
        }

        delete tmpRS;
        // free slot
        slots[j].inUse = false;
    }

    // output matrix
    printDivMat(outF, divvs, true, opts);
    for (size_t i = 0; i < divvs.size(); i++) {
        delete divvs[i];
    }

    // write rarefaction matrices to disk
    if (NoOfMatrices > 0) {
        vector< string > rowNames = Mo->getRowNames();
        if (opts->writeSwap) {
            //printRarefactionMatrix(tmpMatFiles, outF, rareDep, cntsNames, rowNames);
        }
        else {
            //printRarefactionMatrix(MaRare, outF, rareDep, cntsNames, rowNames);
        }
    }

    delete Mo;

    // compute chao2, ACE, ICE and write to file
    vector<vector<mat_fl>> chao2(opts->depth.size());
    vector<mat_fl> ICE;
    vector<mat_fl> ACE;
    computeChao2(chao2, abundInRow);
   // computeCE(ICE, abundInRow);
   // computeCE(ACE, occuencesInRow);
   writeGlobalDiv(opts, ICE, ACE, chao2, outF + "_gDiv.tsv");

    printf("CPU time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    //cout << "Finished\n";
    std::exit(0);
}
else {
    cout << "rtk run mode \"" << mode << "\" undefined.\nSee ./rtk -h for more information.\n";
    //helpMsg();
}



return 0;
}
