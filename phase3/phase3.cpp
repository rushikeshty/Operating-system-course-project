#include <iostream>
#include <cstring>
#include <fstream>
#include<time.h>
#include <unistd.h>
#include<bits/stdc++.h>
#define TIME_QUANTUM 10
#define N 40
#define SIZE 300
#define K 4

using namespace std;

enum TASK{
    IS , OS , LD , RT , WT 
};

enum STATUS{
    Loading , Loaded , Ready , Execute , Terminate , Read , Write
};

class PCB{
    public:
    int ID;
	int TTL; //total time limit
	int TLL; //total line limit 
	int TTC; //total time conuter
	int LLC; //line limit counter
    int TSC; // Time slice counter
    char F;
    int  P_COUNT,D_COUNT;
    int  D , P ,O, END;
    int flag = 0;
    int PTR ;
    short IC;
    string terminateMsg ;
    STATUS status;
    PCB(PCB* pcb)
    {
        ID = pcb->ID;
        TTL = pcb->TTL;
        TLL = pcb->TLL;
        TTC = pcb->TTC;
        LLC = pcb->LLC;
        TSC = pcb->TSC;
        F = pcb->F;
        D = pcb->D;
        P_COUNT= pcb->P_COUNT;
        D_COUNT= pcb->D_COUNT;
        P = pcb->P , END = pcb->END;O = pcb->O;
        status = pcb->status;
        PTR = pcb->PTR;
        IC = pcb->IC;
        terminateMsg = pcb->terminateMsg;
    }
    PCB()
    {
        TTL = TLL = TTC = LLC = TSC  = D = P_COUNT = D_COUNT = P = END= IC =O= ID = 0;
        F = 'P'; // UPCOMING LINE IS PROGRAM CARD
        PTR = -1;
        terminateMsg = ' ';
        status = STATUS::Loading;
    }
};


class CH{
    public:
    int flag;
    int value;
    int time;
    int total_time;
    CH()
    {
        flag = 0 ; value = 0 ; time =  0; total_time=0;
    }
};


typedef struct buffer{
    int status ; // 0 - empty , 1 - ibq , 2-obq
    char value[40];
    void initalize(){ // ZERO SET THE BUFFER
        for(int i =  0 ; i<40 ; i++) //40 BYTES BUFFER SIZE
        {
            value[i] = ' ';
        }
    }
}Buffer;


class MOS{
    private:
    char MainMemory[SIZE][K],DrumMemory[SIZE+200][K], SS[SIZE+1000][K], IR[K], R[K];
    int drum = 0,memory=0;
    short RA ;
    bool C;// TOGGLE REGISTER
    int SI = 0 , PI = 0 , TI = 0 ,IR1=0,IR2=0,IR3=0,IOI = 0;
    CH* ch = new CH[3];
    int count = 0;
    ifstream fi;
    ofstream fo;
    bool mark[30] ;
    int eof = 0;
    TASK task = TASK::IS;
    int flag = 0 ;
    queue<Buffer*> emptyQ,inputfulBufferQ,outputfulBufferQ ;
    queue<PCB*> readyQ,loadQ,inputOutputQ,terminateQ, pendingQ;

    
    void read();
	void write();
    void terminate(int,int);
    int cardReader();
    void startExecution();
    void LR();
    void SR();
    void CR();
    void BT(PCB*);
    void IRI1();
    void IRI2();
    void IRI3();
    void masterMode();
    void store_bufferDrum(Buffer*);
    void store_buffer_DrumMM(PCB*);
    bool check_operand(char , char);
    void simulation();
    void startCH(int);
    void loadDrumMM(PCB*);
    void Map(PCB*,int);
    void execute();
    void round_robin();
    bool mainMeomoryAvailable();
    void drum_to_OutputBuffer(PCB*);

    public:
    MOS();
    void begin();
    void initialization();
    void close_all();
    int Allocate();
    
};


 // ******************* MAIN FUNCTION ************************************// 
int main()
{
	srand(time(0));
    MOS my_os ;
    my_os.begin();
    my_os.close_all();
    return 0;
}
 // ******************* MAIN FUNCTION ************************************// 
 

void MOS::round_robin()
{
    if(readyQ.empty())
        return;
    PCB* temp = readyQ.front();
    readyQ.pop();
    readyQ.push(temp);

}


bool MOS::mainMeomoryAvailable(){
    for(int i =  0 ; i<30 ; i++) // See whether Page is available
    {
        if(!mark[i])
            return true;
    }
    return false;
}

 // ******************* ACTUAL ENTRY ************************************// 
void MOS::begin(){
    int tmr=0; //GLOBAL TIMER
    cout<<"#############  Operating System IS STARTED ############# \n\n";
    initialization();
    IOI = 1; //ch1 start
    do
    {
        cout<<"🕑 GLOBAL TIMER => ["<<tmr<<"] .............."<<endl;
        execute();
        simulation();
        masterMode();
        round_robin();
        tmr++;
        
    }while(eof!=1 ||!inputfulBufferQ.empty() || !outputfulBufferQ.empty() || !loadQ.empty() || !readyQ.empty() || !inputOutputQ.empty() || !terminateQ.empty() );
}


void MOS::initialization(){
    
    memory = 0;
    //initialize MainMemory
    for(int i = 0 ; i<SIZE ; i++)
        for(int j = 0 ; j<K; j++)
            MainMemory[i][j] = ' ';
    
    for(int i =  0 ; i<30 ; i++)
        mark[i] = false; // BOOLEAN ARRAY TO TELL IS PAGE IS AVAILABLE

    for(int i =  0 ; i<K ; i++)
        R[i] = ' ';
    
    // INITIALIZE EMPTY BUFFER QUEUE
    for(int i =  0 ; i<10;i++)
    {
        Buffer* temp = new Buffer;
        temp->initalize(); // SET ENTIRE BUFFER TO BLANK
        temp->status = 0;// 0 - empty , 1 - ibq , 2-obq
        emptyQ.push(temp);
    }
    
    // INITIALZIE DRUM MEMORY
    for(int i =  0 ; i<50;i++)
    {
        for(int j =  0 ; j<K;j++)
        {
            DrumMemory[i][j] = ' ';
        }
    }
    
    // SET CHANNEL VALUES AND TOTAL TIME TAKEN BY THEM
    ch[0].value = 1;
    ch[1].value = 2;
    ch[2].value = 4;
    
    ch[0].total_time = 5;
    ch[1].total_time = 5;
    ch[2].total_time = 2;
    
}

// ALLOCATE PAGES AND MARK THAT IN mark BOOLEAN ARRAY
int MOS::Allocate()
{
    
	int random = rand() % 30;
	while(mark[random])
	{
		random = rand() % 30;
	}
	mark[ random ] = true;
	return random;
}


// OPERAND ERROR IS CHECKED
bool MOS::check_operand(char a , char b) 
{
    if(!( ( (a<='9'&&a>='0') || a==' ') && ( (b<='9'&&b>='0')|| b==' ') )){
        PI = 2;
        return false;
    }
    return true;
}


// ADDRESS TRANS FUNCTION
void MOS::Map(PCB* temp , int VA)
{
    int i = 0 , pte_i ;
	int pte = temp->PTR + (int)VA/10;
    string content;
    for(i = 0 ;i<K;i++)
    { 
        content+=MainMemory[pte][i];
    }
    content[i]='\0';
    if(content == "    ")
    {
        PI = 3;//Page Fault
        cout<<"Page fault=> Setting PI=3 : "<<endl;
        return;
    }
    pte_i = stoi(content);
	RA = pte_i*10 + VA%10;
}


int MOS::cardReader(){
    
    int i = 0;
    char c;
    if(eof == 1 || emptyQ.empty())
    {
        //cout<<"emptyQ is empty or eof = 1"<<endl;
        return 1;
        // ALL R USED
    }
    
    // TAKE ONE FROM 10 EMPTY Buffers
    Buffer* temp = emptyQ.front();
    emptyQ.pop();
    temp->status = 1; //INPUTFULL BQ
    if(inputfulBufferQ.size() == 10)
    {
        cout<<"Input Buffer queue is full ! cant read"<<endl;
        return 1;
    }
    
    if(fi.is_open()) {
        int count = 0;
        fi.get(c);
        if(c == '\n')
            return 0;
        temp->value[count++] = c;
        if(c =='H' && pendingQ.back()->F == 'P'){
                temp->value[count++] = ' ';
                temp->value[count++] = ' ';
                temp->value[count++] = ' ';
        }
        while(fi.good()){
            fi.get(c);
            if(count == 40 || c =='\n'){
                
                if(temp->value[0] == '$') //CONTROL CARD
                {
                    if(temp->value[1] == 'A') //AMJ LINE IS ENCOUNTERED
                    {
                        
                        PCB *temp_pcb = new PCB();
                        temp_pcb->ID = stoi(string(1,temp->value[4])+string(1,temp->value[5])+string(1,temp->value[6])+string(1,temp->value[7]));
                        temp_pcb->TTL = stoi(string(1,temp->value[8])+string(1,temp->value[9])+string(1,temp->value[10])+string(1,temp->value[11]));
                        temp_pcb->TLL = stoi(string(1,temp->value[12])+string(1,temp->value[13])+string(1,temp->value[14])+string(1,temp->value[15]));
                        temp_pcb->TTC = 0;
                        temp_pcb->LLC = 0;
                        temp_pcb->F = 'P';
                        cout<<"$AMJ PCB created for job id : "<<temp_pcb->ID<<" "<<"Next upcoming Card -> "<<temp_pcb->F<<endl; 
                        cout<<"PCB=> TTL:"<<temp_pcb->TTL<<" TLL:"<<temp_pcb->TLL<<" TTC:"<<temp_pcb->TTC<<" LLC:"<<temp_pcb->LLC<<endl;
                        pendingQ.push(temp_pcb);
              
                    }
                    else if(temp->value[1] == 'D'){ //DTA LINE IS ENCOUNTERED
                        pendingQ.back()->F = 'D';
                        cout<<"$DTA Encountered "<<endl;   
                    }
                }
                
                else{
                    if(pendingQ.back()->F == 'P') //program card
                    {            
                        pendingQ.back()->P_COUNT +=1;       
                        cout<<"Encountered Program card. P-Count:"<<pendingQ.back()->P_COUNT<<" PID:"<<pendingQ.back()->ID<<"\n";       
                        //cout<<"Control Card ($AMJ) is in Supervisory Storage. P-cout:"<<pendingQ.back()->P_COUNT<<" PID:"<<pendingQ.back()->ID<<"\n";
                    }
                    else if(pendingQ.back()->F == 'D')//data card
                    {
                        cout<<"Encountered Data card.\n";
                        pendingQ.back()->D_COUNT +=1;
                    }
                    else
                    {
                        cout<<"Invalid value of F"<<endl;
                    }
                }
                
                
                inputfulBufferQ.push(temp);
                if(!( (IOI >> 2) & 1)) // IF 3RD BIT IS NOT SET THEN SET
                {
                    //start channel 3
                    IOI|=4;
                   
                }
              
                task = TASK::IS;
                return 0;
            }
            
            temp->value[count++] = c;
            if(c =='H' && pendingQ.back()->F == 'P'){
                temp->value[count++] = ' ';
                temp->value[count++] = ' ';
                temp->value[count++] = ' ';
            }

            if(fi.eof() && flag == 0)
            {
                inputfulBufferQ.push(temp);
                task = TASK::IS;
                
                cout<<"found-------------"<<endl;
                flag = 1;
            }
        

        }
    }
    return 1;
}

void MOS::store_buffer_DrumMM(PCB* temp){

    //allocate a block and update PT
    int m = Allocate()*10;
    int itr = temp->PTR;
    while(MainMemory[itr][3] != ' ')
    {
        itr++;
    }
    MainMemory[itr][3] = '0'+ (m%100)/10;  
    MainMemory[itr][2] = '0'+ (m/100);
    MainMemory[itr][1] = '0';
    MainMemory[itr][0] = '0';


    for(int i = 0 ; i< 10 ; i++){
        for(int j = 0 ; j<K ; j++){
            MainMemory[m+i][j] = DrumMemory[temp->P+i][j];   
        }
    }
    temp->P+=10;
}


void MOS::loadDrumMM(PCB* temp){
    if(temp->PTR == -1)
    {
        temp->PTR = Allocate()*10;
    } 
    store_buffer_DrumMM(temp);        
}


void MOS::execute(){
    if(readyQ.empty())
    {
        cout<<"No process in Ready Q\n";
        return;
    }
    
    PCB* temp = readyQ.front();
    cout<<"\n Executing Process... "<<endl;
    
    // CHANGE STATUS
    if(temp->status != STATUS::Execute)
    {
        temp->status = STATUS::Execute;
    }
    temp->TSC = 0; // INITIALIZE TIME SLICE COUNT
    
    
    while( (temp->TSC != TIME_QUANTUM) || (temp->IC != (temp->D)) )
    {
        temp->TSC++;
        temp->TTC++;

        //map IC to RA
        Map(temp,temp->IC);
        if(PI)
        {
            break;      
        }
        cout<<"Mapped IC: "<<temp->IC<<" to RA: "<<RA<<endl;
        
        //initilaize IR
        for(int k = 0 ; k<K ; k++){
            IR[k] = MainMemory[RA][k];
        }
        if(IR[0]=='H' && IR[1]==' ')
        {
            SI = 3;
            break;
        }
        
        cout<<"Current TTC: "<<temp->TTC<<endl;
        if(temp->TTC > temp->TTL)
        {
            cout<<"Total Time Limit Exceeded"<<endl;
            TI = 2; 
        }
        
        //INC IC
        temp->IC++;
        
        if(!check_operand(IR[2],IR[3])){
            cout<<"Operand Error"<<endl;
            return;
        }
        
        // ELSE OPERAND IS VALID AND NOW WE CAN PASS IT TO ADDR MAP FUNC
        int operand = stoi(string(1,IR[2])+string(1,IR[3]));
        Map(temp,operand);
        if(PI)
        {
            break;          
        }

        string instruction =  string(1,IR[0])+string(1, IR[1]);
        if(instruction == "LR")
        {
            LR();               
        }               
        else if(instruction == "SR")
        {
            SR();            
        }
        else if(instruction == "CR")
        {
            CR();          
        }
        else if(instruction == "BT")
        {
            BT(temp);
        }
        else if(instruction == "GD")
        {
            cout<<"Setting SI=1 \n";
            SI = 1;
            task = TASK::RT; // UPDATE TASK OF CH3 (READ)
            return;
        }
        else if(instruction == "PD")
        {
            cout<<"Setting SI=2 \n";
            SI = 2;
            task = TASK::WT; // UPDATE TASK OF CH3 (WRITE)
            return;          
        } 
        else if(instruction[0] == 'H')  
        {
            cout<<"Setting SI=3\n";
            SI=3;
            task = TASK::OS;  // UPDATE TASK OF CH3 (OUTPUT SPOOLING)
            return;            
        }
        else
        {
            PI = 1; //operation error
            cout<<"Opcode error: "<<endl;
            return;
        } 
       
    }
    if(temp->TSC == TIME_QUANTUM)
    {
        TI = 1; //Time Slice Out 
    }
}


void MOS::LR(){
    for(int i = 0 ; i<K ; i++){
        R[i] = MainMemory[RA][i];
    }
    
}

void MOS::SR(){
    for(int i = 0 ; i<K ; i++){
        MainMemory[RA][i] = R[i];
    }
}

void MOS::CR(){
    C = true;
    for(int i = 0 ; i<K ; i++){
        if (MainMemory[RA][i] != R[i]){
            C = false;
            return;
        }
    }
}

void MOS::BT(PCB* temp){
    if(C){
        temp->IC = (IR[2]-'0')*10+(IR[3]-'0');
    } 
}

void MOS::drum_to_OutputBuffer(PCB* temp){
    if(emptyQ.empty() || temp->LLC < 0)
        return;
    Buffer* obq ;
    if(temp->LLC == 0)
    {
        cout<<"####### All output lines copied in OutputFull Q with an additonal line -> '"<<temp->terminateMsg<<"' ####### \n\n";
        terminateQ.pop();
        if(!emptyQ.empty())
        {
            obq = emptyQ.front();
            obq->status = 2;
            for(int i =  0 ; i<40;i++)
            {
                obq->value[i] = temp->terminateMsg[i];
            }
            
            outputfulBufferQ.push(obq);
            emptyQ.pop();
        }
        if(emptyQ.size() >=3)
        {
                obq = emptyQ.front();
                emptyQ.pop();
                obq->status = 2;
                outputfulBufferQ.push(obq);
                obq = emptyQ.front();
                emptyQ.pop();
                obq->status = 2;
                outputfulBufferQ.push(obq);
                obq = emptyQ.front();
                emptyQ.pop();
                obq->status = 2;
                outputfulBufferQ.push(obq);
        }
        return;
    }
    obq = emptyQ.front();
    emptyQ.pop();
    obq->status = 2;
    int k = 0;
    for(int i =  0 ; i<10;i++)
    {
        for(int j =  0 ; j<K;j++)
        {
            obq->value[k++] = DrumMemory[temp->O+i][j];
            //free track
            //DrumMemory[temp->O+i][j] = ' ';
        }
    }
    outputfulBufferQ.push(obq);
    temp->O+=10;
    temp->LLC--;

}

void MOS::IRI1(){
    eof = cardReader();
    // 1 FOR EOF ELSE
    cout<<"EOF: "<<eof<<endl;
    if(eof !=1 && !emptyQ.empty())
    {
        startCH(0); //channel 1
    }
}


void MOS::IRI2(){
    if(outputfulBufferQ.empty())
    return;
    Buffer* temp = outputfulBufferQ.front();
    outputfulBufferQ.pop();
    temp->value[39] = '\n';
    fo<<temp->value;
    temp->status = 0;
    temp->initalize();
    emptyQ.push(temp);
    startCH(1);
}

void MOS::IRI3(){
    switch (task)
    {
    case TASK::IS:
    {
        
        if(inputfulBufferQ.empty())
        {
            break;
        }
        if(drum<=490)
        {
            startCH(2);
        }
        else
        {
            cout<<"Drum Memory is full..........."<<endl;
            //exit(0);
        }
        Buffer* temp = inputfulBufferQ.front();
        inputfulBufferQ.pop();
        cout<<"Channel 3 performs IS on : "<<temp->value<<endl;
        if(temp->value[0] == '$')
        {
            cout<<"Encountered Control card: ";
            if(temp->value[1] == 'A')
            {
                cout<<"$AMJ\n\nLoading new Job into Supervisory storage...!!!\n";
                pendingQ.front()->F = 'P';
                pendingQ.front()->P = drum;
            }
            else if(temp->value[1] == 'D'){
                pendingQ.front()->F = 'D';
                pendingQ.front()->D = drum;
                cout<<"$DTA"<<endl;   
            }
            else //$END
            {
                pendingQ.front()->O = drum;
                pendingQ.front()->END = drum - 10;
                pendingQ.front()->status = STATUS::Loaded;
                PCB* temp = new PCB(pendingQ.front());
                loadQ.push(temp);
                task = TASK::LD;
                cout<<"$END"<<endl<<endl; 
                cout<<"#######  INPUT SPOOLING DONE  ####### \n";
                cout<<"PCB of Job id : "<<pendingQ.front()->ID<<endl;
                drum += (pendingQ.front()->TLL)*10; 

                cout<<"\nJob added to Load queue PID:"<<pendingQ.front()->ID<<"\n";
                pendingQ.pop();
                // all loaded in drum ,ready to load
            }
        }
        else //other than control card, store to drum
        {
            store_bufferDrum(temp);
        }
        temp->status = 0;
        temp->initalize();
        emptyQ.push(temp);
    break;
    }
    
    
    case TASK::LD:
    {
        if(loadQ.empty())
            break;
        PCB* temp = loadQ.front();
        loadDrumMM(temp);
        temp->P_COUNT--;
        startCH(2);
        if(temp->P_COUNT == 0) // ALL PROGRAM CARDS LOADED
        {
            temp->status = STATUS::Ready;
            readyQ.push(temp);
            loadQ.pop();
        }
    break;
    }
    
    case TASK::RT:// FOR GD
    {
        
        if(inputOutputQ.empty())
            break;
        PCB* temp = inputOutputQ.front();
        if(temp->D_COUNT == 0) // OUT OF DATA ERROR
        {
            terminateQ.push(temp);
            inputOutputQ.pop();
            
            terminate(1,-1);

            break;
        }
        startCH(2); //CH3
        for(int i = 0 ; i< 10 ; i++){
            for(int j = 0 ; j<K ; j++){
                MainMemory[RA+i][j] = DrumMemory[temp->D+i][j];
            }
        }// DRUM TO MM
        temp->D+=10; // points to next data card
        temp->D_COUNT--;
        temp->TSC = 0;
        readyQ.push(temp);
        inputOutputQ.pop();
    break;
    }
    
    
    case TASK::WT: //PD
    {
        if(inputOutputQ.empty())
            break;
        PCB* temp = inputOutputQ.front();
        temp->LLC++;
        if(temp->LLC > temp->TLL) //line limit error
        {
            temp->LLC--;
            terminateQ.push(temp);
            inputOutputQ.pop();
            terminate(2,-1);
            break;
        }
        startCH(2);
        for(int i = 0 ; i< 10 ; i++){
            for(int j = 0 ; j<K ; j++){
                DrumMemory[temp->END+10+i][j] = MainMemory[RA+i][j];
            }
        }
        temp->END+=10;
        temp->TSC = 0;
        if(temp->status == STATUS::Terminate)// DO PD AFTER TLE
        {
            terminateQ.push(temp);
            terminate(3,-1);   
        }
        else
            readyQ.push(temp);
        	inputOutputQ.pop();
    break;   
    }
    
    case TASK::OS:
    {
        if(terminateQ.empty())
            break;
        startCH(2);
        PCB* temp = terminateQ.front();
        drum_to_OutputBuffer(temp);
        if(!( (IOI >> 1) & 1))
        {
            // IOI+=2;
            IOI|=2; // activate channel 2
        }

    break;   
    }
    
    default:
    {
        break;
    }
    }


    //priority wise
    if(!terminateQ.empty())
    {
        task = TASK::OS;
    }
    else if(!inputfulBufferQ.empty() && drum <=490)
    {
        task = TASK::IS;
    }
    else if(!loadQ.empty())
    {
        task = TASK::LD;
    }
    else if(!inputOutputQ.empty())
    {
        PCB* temp = inputOutputQ.front();
        if(temp->status == STATUS::Read)
        {
            task = TASK::RT;
        }
        else
        {
            task = TASK::WT;
        }
    }

}
void MOS::masterMode(){
    
    if(!readyQ.empty() &&( PI || SI || TI))
    {
    PCB* temp = readyQ.front();
    if(TI == 0 || TI == 1) // NO TLE
    {
       
        if(PI == 1)
        {
            terminateQ.push(temp);
            readyQ.pop();
            terminate(4,-1);
        }
        else if(PI == 2)
        {
            terminateQ.push(temp);
            readyQ.pop();
            terminate(5,-1);
        }
        else if(PI == 3)
        {
            string instruction = string(1,IR[0])+string(1, IR[1]);
            //invalid page fault
            if(instruction=="PD" || instruction=="LR" || instruction=="CR") 
            {
                terminateQ.push(temp);
                readyQ.pop();
                terminate(6,-1);      
            }                     
            else
            {
                // Valid Page Fault
                int m = Allocate()*10;

                int itr = temp->PTR+stoi(string(1,IR[2])+string(1, IR[3]))/10;
                //updated page table
                MainMemory[itr][3] = '0'+ (m%100)/10;  
                MainMemory[itr][2] = '0'+ (m/100);
                MainMemory[itr][1] = '0';
                MainMemory[itr][0] = '0';  
                temp->IC--;
                PI = 0;
                return;        

            }            	    
        }
        
        
        else if(SI == 1) // READ => (READYQ -> IOQ)
        {
            temp->status = STATUS::Read;
            inputOutputQ.push(temp);
            readyQ.pop();
            task = TASK::RT; //CHANGE TASK
            SI = 0;
        }
        else if(SI == 2) // WRITE => (READYQ -> IOQ)
        {
            temp->status = STATUS::Write;
            inputOutputQ.push(temp);
            readyQ.pop();
            task = TASK::WT; //CHANGE TASK
            SI = 0;
        }
        else if(SI == 3)
        {
            //cout<<"#########  Job Executed Successfully  #########\n\n";
            //cout<<"######### Output Spooling Starts ########## \n\n";
            terminateQ.push(temp);
            readyQ.pop();
            terminate(0,-1);   
            SI = 0;
        }   

    }
    else
    {
        // CASE TI=2 AND PI
        if(PI == 1)
        {
            terminateQ.push(temp);
            readyQ.pop();
            terminate(3,4);      
        }
        else if(PI == 2)
        {
            terminateQ.push(temp);
            readyQ.pop();
            terminate(3,5);      
        }        
        else if(PI ==3)
        {
            terminateQ.push(temp);
            readyQ.pop();
            terminate(3,-1);      
        }        
        
        
        // CASE TI=2 AND SI
        else if(SI == 1)
        {
            terminateQ.push(temp);
            readyQ.pop();
            terminate(3,-1);      
        }          
        else if(SI == 2)
        {
            //write
            readyQ.pop();
            task = TASK::WT;
            temp->status = STATUS::Write;
            inputOutputQ.push(temp);
            temp->status = STATUS::Terminate;
        }
        else if(SI == 3)
        {
            terminateQ.push(temp);
            readyQ.pop();
            terminate(0,-1);          
        }

    }  
    }
        
    
    if( (IOI >> 2) & 1) // IF THIRD BIT IS SET ACTIVATE CH3
        IRI3();
    if( (IOI >> 1) & 1) // IF SECOND BIT IS SET ACTIVATE CH2
        IRI2();
    if( (IOI >> 0) & 1) // IF FIRST BIT IS SET ACTIVATE CH1
        IRI1();

}

void MOS::store_bufferDrum(Buffer* temp){
    int k= 0;
    cout<<"storing at : "<<drum<<endl; 
    for(int i = 0 ; i< 10 ; i++){
        for(int j = 0 ; j<K ; j++){
            if(temp->value[k]=='\0')
                break;
            DrumMemory[drum+i][j] = temp->value[k++];   
        }
    }
    drum+=10;
    
}

MOS::MOS(){
    fi.open("Correct1.txt",ios::in); 
    fo.open("output.txt",ios::out); 
    memset(DrumMemory,' ', sizeof(DrumMemory));
}

void MOS::close_all(){
    fi.close();
    fo.close();
}

void MOS::terminate(int EM1 , int EM2){
    task = TASK::OS; 
    if(terminateQ.empty())
        return;
    PCB* temp = terminateQ.back();
    
    temp->status = STATUS::Terminate;
    string msg ;
    if(EM1 == 0)
        msg = "ALL GOOD 👍";
    else if(EM1 == 1)
        msg="OUT OF DATA ERROR ❌";
    else if(EM1 == 2)
        msg="LINE LIMIT EXCEEDED ❌";
     else if(EM1 == 6)
        msg="INVALID PAGE FAULT ❌";
     else if(EM1 == 4)
        msg="OPCODE ERROR ❌";
     else if(EM1 == 5)
        msg="Operand Error ❌";
     else {
        msg= "TLE Error & ❌";
        if(EM2 == -1)
        msg="Time Limit Exceeded Error ❌";
        else if(EM2 == 4)
            msg+=" Opcode Error ❌";
        else 
            msg+=" Operand Error ❌";      
     }
     temp->terminateMsg = msg;
     //cout<<"Terminate msg: "<<temp->terminateMsg<<" is added to output file"<<endl;
     fo<<"SI: "<<SI<<" TI: "<<TI<<" PI: "<<PI<<"\n";
	 fo<<"IC: "<<temp->IC<<" TTC: "<<temp->TTC<<" TTL: "<<temp->TTL<<" LLC: "<<temp->LLC<<" TLL: "<<temp->TLL<<"\n\n";	
     //fo<<"IC: "<<PCB::IC<<" TTC: "<<PCB::TTC<<" TTL: "<<PCB::TTL<<" LLC: "<<PCB::LLC<<" TLL: "<<PCB::TLL<<"\n";
     PI = 0 ; SI = 0 ; TI = 0;
} 

void MOS::simulation()
{
    for(int i =  0 ; i<3 ; i++)
    {
        if(ch[i].flag) //ith CHANNLE IS ACTIVE
        {        
            ch[i].time++;

            if(ch[i].time == ch[i].total_time)
            {
                cout<<"Channel No "<<i+1<<" is done \n"<<endl;

                IOI|= ch[i].value;
                ch[i].flag = 0; //set the flag and free that channel
            }
        }
    }
}
// DONE

void MOS::startCH(int channel){
    
    IOI &= (7-ch[channel].value);
    ch[channel].time = 0;
    ch[channel].flag = 1; //busy
    cout<<"Start Channel "<<channel+1<<" Current IOI value: "<<IOI<<endl;

}