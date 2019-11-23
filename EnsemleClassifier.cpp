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
#include <unistd.h> 

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
        const char* c = "0";
        return c;
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

int get_csv_rows_count(const char * path){
    int fd = open(path, O_RDWR);
    if(fd < 0) {
        cout << "error: failed to open file " << path << endl;
        exit(1);
    }

    ifstream file(path);
    int counter = 0;
    string line = "";
    while(getline(file, line))
       counter++;
    return counter;
}

float strtod(string w)
{
    double w_int;
    istringstream ss(w);
    ss >> w_int;
    return w_int;
}

int read_from_voter_named_pipe(int row_index, char *classifier_name){
    const char* pipe_name = strcat(classifier_name, itconstc(row_index)); 
    ifstream infile(pipe_name, ios::out);
    if(!infile) {
      cout << "error: can'nt open file to read!" << endl;
      return -1;
    }
    return 0;
}

int write_to_voter_named_pipe(int row_index, char* classifier_name, int result_index){
    const char* pipe_name = strcat(classifier_name, itconstc(row_index));
    ofstream outfile (pipe_name, ios::out);
    if(!outfile) {
      cout << "error: can'nt open file to write!" << endl;
      return -1;
    }
    const char* c = itconstc(result_index);
    outfile.write (c, strlen(c));
    outfile.close();
    return 1;
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
                cout<<"error: failed to read pipe"<<endl;
                close(fd[i][0]);
                exit(1);
            }else{
                close(fd[i][0]);
                std::vector<std::vector<string>> rows, weights;
                const char* tmp = ".csv";
                weights = readcsv(strcat(weight_vectors_path, strcat(classifier_name, tmp)));
                rows = readcsv(strcat(validation_path, "/dataset.csv"));
                int row_index = 0;
                for (std::vector<string> row : rows)
                {
                    double max_result_value = -1000;
                    double result_value = -1;
                    int result_index = -1;
                    int w_index = 0;
                    for (std::vector<string> weight : weights)
                    {
                        int w_col = 0;
                        for (string w : weight)
                        {
                            int row_col = 0;  
                            for (string data : row)
                            {
                                if (row_col == w_col)
                                {
                                    result_value += strtod(data) * strtod(w);
                                }else if(w_col == weight.size() - 1){
                                    result_value += strtod(w);
                                }                                
                                row_col++;
                            }
                            w_col++;
                        }
                        if (max_result_value < result_value)
                        {
                            max_result_value = result_value;
                            result_index = w_index;
                        }
                        w_index++;
                    }
                    write_to_voter_named_pipe(row_index, classifier_name, result_index);
                    row_index++;
                }
            }
            exit(0);
        }

        close(fd[i][0]);
        string sd = "/classifier_" + itos(i);
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
        int rows_count = get_csv_rows_count(strcat(validation_path, "/dataset.csv"));
        int *labels[rows_count];
        for(int i = 0; i < rows_count; i++){
            labels[i] = new int[n];
            for (int j = 0; j < n; ++j)
            {
                string tmp_s = "/classifier_" + itos(j) + ".csv";
                char char_array[tmp_s.length() + 1]; 
                strcpy(char_array, tmp_s.c_str());
                int result = read_from_voter_named_pipe(i, char_array);
                cout << result << endl;
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