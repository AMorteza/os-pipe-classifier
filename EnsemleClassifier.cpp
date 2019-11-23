#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <dirent.h>
#include <vector>
#include <iterator>
#include <algorithm>

//pipe
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

//fork
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

string itos(int number)
{
    if (number == 0)
    {
        return "0";
    }
    string snum;
    while(number > 0){
        snum = char(number%10 + '0') + snum;
        number = number/10;
    }
    return snum;
}


const char* itconstc(int number)
{
    if (number == 0)
    {
        return "0";
    }
    const char* snum;
    while(number > 0){
        snum = char(number%10 + '0') + snum;
        number = number/10;
    }
    return snum;
}

char itoc(int number)
{
    if (number > 9)
    {
        cout << "error in conver classifier id to char" << endl;
        exit(1);
    }
    
    if (number == 0)
        return '0';
    else if(number == 1)
        return '1';
    else if (number == 2)
        return '2';
    else if (number == 3)
        return '3';
    else if (number == 4)
        return '4';
    else if (number == 5)
        return '5';
    else if (number == 6)
        return '6';
    else if (number == 7)
        return '7';
    else if (number == 8)
        return '8';
    else if (number == 9)
        return '9';
}

int concatint(int x, int y) {
    int pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
} 

int read_from_voter_named_pipe(int dataset_row, int classifier){
    const char* pipe_name = itconstc(concatint(dataset_row, classifier)); 
    int in_fd = open(pipe_name, O_RDWR);
    if(in_fd < 0) {
        cout<<"error in load named pipe" << endl;
        exit(1);
    }
    int result;
    int n;
    char buf[2];
    if ((n = read(in_fd, buf, 2)) > 0)
    {
        result = atoi(buf);
    }
    close(in_fd);
    return result;
}

int write_to_voter_named_pipe(int dataset_row, int classifier, int result){
    int out_fd;
    string value = itos(result);
    const char* pipe_name = itconstc(concatint(dataset_row, classifier));  
    if ( (out_fd = open(pipe_name, O_WRONLY)) < 0){
        cout<<"error: failed to load named pipe" << endl;
        close(out_fd);
        return 0;
    }
    if (write(out_fd,value.c_str(), value.size()+1) != value.size()+1) { 
        close(out_fd);
        return 1;
    }
    close(out_fd);
}

int get_number_of_files_in_dir(char * path){
    DIR *dp;
    int counter = 0;
    struct dirent *ep;
    dp = opendir(path);
    if (dp != NULL)
    {
        while(ep = readdir(dp)){
            if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..") != 0)
                counter++;
        }
        (void) closedir(dp);
    }else{
        cout << "error: failed to open " << path << endl;
        exit(1);
    }
    return counter;
}

vector<std::vector<string>> readcsv(const char * path){
    int fd = open(path, O_RDWR);
    if(fd < 0) {
        cout << "error: failed to open file " << path << endl;
        exit(1);
    }

    ifstream file(path);
    std::vector<std::vector<string>> dataList;
    string word, line = "";
    while(getline(file, line))
    {
        stringstream s(line);
        std::vector<string> v;
        while(getline(s, word, ','))
            v.push_back(word);
        dataList.push_back(v);
    }
    return dataList;
}

int main(int argc, char *argv[])
{
    char* validation_path = argv[1];
    char* weight_vectors_path = argv[2];
    int n = get_number_of_files_in_dir(weight_vectors_path);
    int *fd[10]; 
    for(int i =0; i < n; i++)
        fd[i] = new int[2];

    for(int i = 0; i < 10; i++){
        if(pipe(fd[i]) == -1){
            cout << "error: failed to creating pipe" << endl;
            return 1;
        }
        int classifier_pid = fork();
        if (classifier_pid == 0) {
            close(fd[i][1]);
            char classifier_name[100];
            if (read(fd[i][0] ,&classifier_name ,100*sizeof(char)) <= 0) {
                cout<<"error in read pipe!!"<<endl;
                close(fd[i][0]);
                exit(1);
            }else{
                close(fd[i][0]);
                std::vector<std::vector<string>> rows, weights;
                weights = readcsv(strcat(weight_vectors_path, classifier_name));
                rows = readcsv(strcat(validation_path, "/dataset.csv"));
                for (std::vector<string> row : rows)
                {
                    int max = -1;
                    int result = -1;
                    int weight_value_row_index = 0;
                    for (std::vector<string> weight : weights)
                    {
                        int value = -1;
                        int weight_value_col_index = 0;
                        for (string w : weight)
                        {
                            stringstream convert_weight(w);
                            int row_data_col_index = 0;
                            for (string data : row)
                            {
                                stringstream convert_data(data);
                                if (row_data_col_index == weight.size() - 1) //BIAS
                                {
                                    int w = 0;
                                    convert_weight >> w;
                                    result+= w;     
                                }else if (weight_value_col_index == row_data_col_index) //BETA
                                {
                                    int d = 0;
                                    convert_data >> d;
                                    int w = 0;
                                    convert_weight >> w;
                                    result += w * d; 
                                }
                                row_data_col_index++;
                            }
                            weight_value_col_index++;
                        }
                        if (max < value)
                        {
                            max = value;
                            result = weight_value_row_index;
                        }
                        weight_value_row_index++;
                    }                    
                }
                // 1- open dataset.csv file
                // 2- foreach rows as row
                //  2-1 calculate row weight result  
                //  2-2 write result to voter named pipe
            }
            exit(0);
        }

        close(fd[i][0]);
        string sd = "/classifier_" + itos(i) + ".csv";
        if (write(fd[i][1], sd.c_str(), sd.size()+1) != sd.size()+1) {
            close(fd[i][1]);
            exit(1);
        }
        close(fd[i][1]);
    }

    for(int i = 0; i < 10; i++)
        wait(NULL);
    cout << "linear classifiers sub-proccess are done." << endl;

    int voter_pid = fork();
    if (voter_pid == 0)
    {
        int *labels[1001];
        for(int i = 0; i < 1001; i++){
            labels[i] = new int[10];
            for (int j = 0; j < 10; ++j)
            {
                int result = 0;//read_from_voter_named_pipe(i, j);
                labels[i][j] = result;  
            }
        }
        //  1- foreach lables rows as row
        //      1-1 find most repeated as result
        //          1-1-1 pass result to parent process with named pipe with 
        exit(0);
    }
    wait(NULL);
    cout << "voter sub-proccess is done." << endl;
    
    float count = 0;
    //  1- foreach lables.csv rows as row => value     
    //      1-2 comare results[row] with value
    //          1-2-1 if equal count++  
    cout << "Accuracy: "<< count/1002 << endl; 

    return 0;
}