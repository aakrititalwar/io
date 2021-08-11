#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>
#include <cstring>
#include <cctype>
#include <string>
#include <bits/stdc++.h>
#include <queue>
using namespace std;

struct IO_req{
    int arrival_time;
    int track;
    int start_time = -1;
    int end_time = -1;
    int wait_time = -1;
};

FILE* inpfile;
static char linebuf[1024];
vector<IO_req*> input_io_vec;
list<IO_req*> io_wait_queue;
list<IO_req*> list_a;
list<IO_req*> list_b;
list<IO_req*>* active_queue;
list<IO_req*>* add_queue;
bool cond_check;

int CURRENT_TIME = 0;
int curr_track = 0;
int io_req_counter = 0;
const char* DELIM = " \t\n\r\v\f";
IO_req* CURRENT_ACTIVE_IO = NULL;
int flow = 0;
int tot_movement = 0;
//int maxwtt = -1;


class ioScheduler{
    public:
    virtual IO_req* getnextIO_from_waitqueue(){
        cout << "NO scheduler type specified" << endl;
        return NULL;
    }
    virtual void add_to_queue(IO_req* newIO){
        io_wait_queue.push_back(newIO);
    }
    virtual bool isFLOOK(){
        return false;
    }

};

class FIFO : public ioScheduler{
    public:
    IO_req* getnextIO_from_waitqueue(){
        IO_req* i = io_wait_queue.front();
        io_wait_queue.pop_front();
        if(i->track > curr_track){
            flow = 1;
            }
        else{
            flow = -1;
            }
        return i;
    }

};

class SSTF : public ioScheduler{
    public :
    IO_req* getnextIO_from_waitqueue(){
        IO_req* NEXT_IO = NULL;
        int min_diff = 100000;
        int pos = -1;
        int min_pos;
        list<IO_req*>::iterator i = io_wait_queue.begin();
        list<IO_req*>::iterator i2 = io_wait_queue.begin();
        while(i != io_wait_queue.end()){
            pos++;
            if(NEXT_IO == NULL){
                NEXT_IO = *i;
                min_diff = abs(curr_track - (*i)->track);
                min_pos = pos;
            }
            else{
                if(abs(curr_track - (*i)->track) < min_diff){
                    min_diff = abs(curr_track - (*i)->track);
                    NEXT_IO = *i;
                    min_pos = pos;
                }
            }
            i++;        
            }
        
        if(NEXT_IO->track > curr_track){
            flow = 1;
        }
        else{
            flow = -1;
        }
        advance(i2, min_pos);
        io_wait_queue.erase(i2);
        return NEXT_IO;
    }

};

class LOOK : public ioScheduler{
    public:
    IO_req* getnextIO_from_waitqueue(){
        list<IO_req*>::iterator i = io_wait_queue.begin();
        list<IO_req*>::iterator i2 = io_wait_queue.begin();
        IO_req* NEXT_IO = NULL;
        int min_diff = 100000;
        int pos = -1;
        int min_pos;
        //cout << "flow" << flow << endl;
        if(flow == 0 || flow == 1){
        while(i != io_wait_queue.end()){
            pos++;
            if(((*i)->track >= curr_track) && (((*i)->track - curr_track) < min_diff)){
                    NEXT_IO = *i;
                    min_pos = pos;
                    min_diff = (*i)->track - curr_track;
            }
            i++;
        }
       //cout << "yay" << endl;
        if(NEXT_IO != NULL){
            //cout << "hi" << endl;
            advance(i2, min_pos);
            io_wait_queue.erase(i2);
            flow = 1;
            return NEXT_IO;
        }
        }
        
        if(NEXT_IO == NULL || flow == -1){
            //cout << "hey" << endl;
            min_diff = 10000;
            pos = -1;
            i = io_wait_queue.begin();
            i2 = io_wait_queue.begin();
            while(i != io_wait_queue.end()){
            pos++;
            if((*i)->track <= curr_track && ((curr_track - (*i)->track) < min_diff)){
                    NEXT_IO = *i;
                    min_pos = pos;
                    min_diff = curr_track - (*i)->track;
            }
            i++;
            }
            if(NEXT_IO != NULL){
                advance(i2, min_pos);
                io_wait_queue.erase(i2);
                flow = -1;
                return NEXT_IO;
            }
            else{
                flow = 1;
                return this->getnextIO_from_waitqueue();
            }

        }
        
    }



};

class CLOOK : public ioScheduler{
    public:

    IO_req* get_smallest(){
        list<IO_req*>::iterator itr = io_wait_queue.begin();
        list<IO_req*>::iterator itr2 = io_wait_queue.begin();
        IO_req* NEXT_IO = io_wait_queue.front();
        int pos = -1;
        int min_pos = 0;
        while(itr != io_wait_queue.end()){
            pos++;
            if((*itr)->track < NEXT_IO->track){
                NEXT_IO = (*itr);
                min_pos = pos;
            }
            itr++;
        }
        advance(itr2, min_pos);
        io_wait_queue.erase(itr2);
        //cout << NEXT_IO->track << endl;
        flow = -1;
        return NEXT_IO;
    }

    IO_req* getnextIO_from_waitqueue(){
        list<IO_req*>::iterator i = io_wait_queue.begin();
        list<IO_req*>::iterator i2 = io_wait_queue.begin();
        IO_req* NEXT_IO = NULL;
        int min_diff = 100000;
        int pos = -1;
        int min_pos;
        while(i != io_wait_queue.end()){
            pos++;
            if(((*i)->track >= curr_track) && (((*i)->track - curr_track) < min_diff)){
                NEXT_IO = *i;
                min_pos = pos;
                min_diff = (*i)->track - curr_track;
            }
            i++;
        }

        if(NEXT_IO != NULL){
            flow = 1;
            advance(i2, min_pos);
            io_wait_queue.erase(i2);
            //cout << NEXT_IO->track << endl;
            return NEXT_IO;
        }
        else{
            return this->get_smallest();
        }



    }
    

};

class FLOOK : public ioScheduler{
    public :
    FLOOK(){
        active_queue = &list_a;
        add_queue = &list_b;
    }
    void add_to_queue(IO_req* newIO){
        
        add_queue->push_back(newIO);
    }
    void switch_queue(){
        list<IO_req*>* temp;
        temp = active_queue;
        active_queue = add_queue;
        add_queue = temp;
    }
    bool isFLOOK(){
        return true;
    }
    IO_req* getnextIO_from_waitqueue(){ 
        list<IO_req*>::iterator i = active_queue->begin();
        list<IO_req*>::iterator i2 = active_queue->begin();
        IO_req* NEXT_IO = NULL;
        int min_diff = 100000;
        int pos = -1;
        int min_pos;
        if(!active_queue->empty()){
            if(flow == 0 || flow == 1){
                while(i != active_queue->end()){
                pos++;
                if(((*i)->track >= curr_track) && (((*i)->track - curr_track) < min_diff)){
                        NEXT_IO = *i;
                        min_pos = pos;
                        min_diff = (*i)->track - curr_track;
                }
                i++;
        }
       //cout << "yay" << endl;
        if(NEXT_IO != NULL){
            //cout << "hi" << endl;
            advance(i2, min_pos);
            active_queue->erase(i2);
            flow = 1;
            return NEXT_IO;
            }
            
            }
            if(NEXT_IO == NULL || flow == -1){
            //cout << "hey" << endl;
            min_diff = 10000;
            pos = -1;
            i = active_queue->begin();
            i2 = active_queue->begin();
            while(i != active_queue->end()){
            pos++;
            if((*i)->track <= curr_track && ((curr_track - (*i)->track) < min_diff)){
                    NEXT_IO = *i;
                    min_pos = pos;
                    min_diff = curr_track - (*i)->track;
            }
            i++;
            }
            if(NEXT_IO != NULL){
                advance(i2, min_pos);
                active_queue->erase(i2);
                flow = -1;
                return NEXT_IO;
            }
            else{
                //cout << "YAAYYYYYY" << endl;
                flow = 1;
                return this->getnextIO_from_waitqueue();
            }

        }
        
        }
        else{
            //cout << flo
            this->switch_queue();
            this->getnextIO_from_waitqueue();

        }



    }

};



ioScheduler* IO_SCHEDULER;



int parseInput(int argc, char *argv[]) {
    int flag;
    int nofarg = 0;
    while ((flag = getopt(argc, argv, "s:")) != -1) {
        nofarg++;
        char temp;
        char schedAlgo;
        if (flag == 's') {
            // cout << optarg << endl;
            schedAlgo = optarg[0];
            if (schedAlgo == 'i') {
                IO_SCHEDULER = new FIFO();
            }
            else if (schedAlgo == 'j') {
                IO_SCHEDULER = new SSTF();
            }
            else if (schedAlgo == 's') {
                IO_SCHEDULER = new LOOK();
            }
            else if (schedAlgo == 'c') {
                IO_SCHEDULER = new CLOOK();
            }
            else if (schedAlgo == 'f') {
                IO_SCHEDULER = new FLOOK();
            }
            //else if (schedulerType == 'R') {
            //     THE_SCHEDULER = new RR();
            //     sscanf(optarg, "%c%d", &temp, &time_quantum);
            // } else if (schedulerType == 'P') {
            //     sscanf(optarg, "%c%d:%d", &temp, &time_quantum, &maxprio);
            //     THE_SCHEDULER = new PRIO();
            // } else if (schedulerType == 'E') {
            //     sscanf(optarg, "%c%d:%d", &temp, &time_quantum, &maxprio);
            //     THE_SCHEDULER = new PREPRIO();
            //  }
        } else {
            cout << "Invalid Argument";
            exit(0);
        }
    }
    return nofarg;
    // read the input files
}

void print_vec(){
    for (vector<IO_req*>::iterator i = input_io_vec.begin();
         i != input_io_vec.end();
         i++)
        cout << (*i)->arrival_time << " ";
  
    cout << endl;

}


void readinpfile(){
    while(fgets(linebuf,1024, inpfile)){
        if(linebuf[0] == '#')
        continue;
        IO_req* IO = new IO_req();
        char* at = strtok(linebuf, DELIM);
        int arrtime = atoi(at);
        IO->arrival_time = arrtime;
        char* tr = strtok(NULL, DELIM);
        int track = atoi(tr);
        IO->track = track;
        input_io_vec.push_back(IO);
    }
    //print_vec();
}

int get_next_arrival_time(){
    if(io_req_counter >= input_io_vec.size()){
        return -1;
    }
    else{
        IO_req* i = input_io_vec.at(io_req_counter);
        return i->arrival_time;
    }

}

void strategy(){
    int i = 0;
    while(true){
        int NAT = get_next_arrival_time();
        //cout << "NAT" << NAT << endl;
        if(NAT == CURRENT_TIME){
            //cout << "yo" << endl;
            IO_req* newIO = input_io_vec.at(io_req_counter);
            IO_SCHEDULER->add_to_queue(newIO); 
            //cout << wctype(add_queue) << endl;
            //cout << (add_queue->front())->track;
            //cout << "lame" << endl;
            //cout << newIO->track << endl;
            //cout << CURRENT_TIME <<": " << io_req_counter << " ADD " << newIO->track << endl;    
            io_req_counter++;   
        }
        if(CURRENT_ACTIVE_IO != NULL && curr_track == CURRENT_ACTIVE_IO->track){
            CURRENT_ACTIVE_IO->end_time = CURRENT_TIME;
            //cout << "EnD " << CURRENT_TIME << endl;
            CURRENT_ACTIVE_IO  = NULL;
        }
        if(CURRENT_ACTIVE_IO == NULL){
            if(IO_SCHEDULER->isFLOOK()){
                cond_check = !(add_queue->empty() && active_queue->empty());
            }
            else {
                cond_check = !io_wait_queue.empty();
            }
            if(cond_check){
                IO_req* io = IO_SCHEDULER->getnextIO_from_waitqueue();
                CURRENT_ACTIVE_IO = io;
                io->start_time = CURRENT_TIME;
                //cout << CURRENT_TIME <<": " << io_req_counter << " ISSUE " << io->track <<curr_track << endl;
                if(io->track == curr_track){
                    continue;
                }
                // if(io->track > curr_track){
                //     flow = 1;
                // }
                // else if(io->track == curr_track){
                //     flow = 0;
                //     continue;
                // }
                // else{
                //     flow = -1;
                // }
                 
            }
            else if(get_next_arrival_time() == -1){
                //cout << "Exit" << endl;
                break;
            }
        }
        if(CURRENT_ACTIVE_IO != NULL){
            curr_track = curr_track + flow;
            tot_movement = tot_movement + 1;
            //cout << "CT" << curr_track;
        }
        CURRENT_TIME = CURRENT_TIME + 1;
        i++;
    }
} 

double cal_tat(){
    double avgtat;
    int sum = 0;
    for (int i = 0; i < input_io_vec.size(); i++){
        IO_req* io = input_io_vec.at(i);
        sum = sum + (io->end_time - io->arrival_time);
    }
    avgtat = (double)sum/(input_io_vec.size());
    return avgtat;
}

double cal_avgWT(){
    //cout << "IN WTT";
    double avgWT;
    int sum = 0;
    for (int i = 0; i < input_io_vec.size(); i++){
        IO_req* io = input_io_vec.at(i);
        io->wait_time = (io->start_time - io->arrival_time);
        sum = sum + io->wait_time;
    }
    avgWT = (double)sum/(input_io_vec.size());
    return avgWT;
}

int get_maxWT(){
    int maxwtt = -1;
    int sum = 0;
    for (int i = 0; i < input_io_vec.size(); i++){
        IO_req* io = input_io_vec.at(i);
        io->wait_time = (io->start_time - io->arrival_time);
        sum = sum + io->wait_time;
        if(io->wait_time > maxwtt){
            maxwtt = io->wait_time;
        }
    }
    return maxwtt;
}





void print_stats(){
    vector<IO_req*>::iterator i = input_io_vec.begin();
    int pos = -1;
    while(i != input_io_vec.end()){
        pos++;
        printf("%5d: %5d %5d %5d\n",pos, (*i)->arrival_time, (*i)->start_time, (*i)->end_time);
        //cout << pos <<":    " << (*i)->arrival_time << "    " << (*i)->start_time << "      " << (*i)->end_time << endl;
        i++;
    }
    printf("SUM: %d %d %.2lf %.2lf %d\n",
            CURRENT_TIME, tot_movement, cal_tat(), cal_avgWT(), get_maxWT());
}
// void strategy(){
//     while(true){
//         time_counter++;
//         IO_req* next_io_req = input_io_vec.at(io_req_counter);
//         if(next_io_req->arrival_time == time_counter){
//             io_wait_queue.push(next_io_req);
//         }
//         if(CURRENT_ACTIVE_IO == NULL){
//             IO_req* io = getnextIO_from_waitqueue();
//             CURRENT_ACTIVE_IO = io;
//             io->start_time = time_counter;

//         }
//         curr_track = curr_track + flow;
//         if(CURRENT_ACTIVE_IO != NULL){

//             if(curr_track == CURRENT_ACTIVE_IO->track){
//                 CURRENT_ACTIVE_IO->end_time = time_counter;
//                 CURRENT_ACTIVE_IO = NULL;
//                 if(next_io_req>= input_io_vec.size())
//                 {
//                     break;
//                 }
//             }


            
//         }

//     }
// }


int main(int argc, char *argv[]){
    int nofarg = parseInput(argc, argv);
    inpfile = fopen(argv[nofarg+1], "r");
    readinpfile();
    // IO_req* io = input_io_vec.at(io_req_counter);
    // cout << io->track;
    // cout << io->arrival_time;
    // add_queue->push_front(io);
    strategy();
    //cout << endl;
    print_stats();
}