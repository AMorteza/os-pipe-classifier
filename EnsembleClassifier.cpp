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
#include <iomanip>

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
    int c = 0;
    while(getline(file, line))
    {
    	if(c == 1){    
             stringstream s(line);
             std::vector<string> v;
             while(getline(s, word, ','))
                v.push_back(word);
             dataList.push_back(v);
    	}
	   c = 1;
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

long double strtol(string item, int w_col)
{
    long double d;
    istringstream(item) >> d;
    return d;
}

int read_from_named_pipe(const char* pipe_name){
    string str = "./pipe_";
    char *cstr = &str[0];
    strcat(cstr, pipe_name);
    FILE* fp = fopen(cstr, "r");
    char line[256];
    int num = 0;
    while ( fgets(line, 255, fp) != NULL )
        sscanf(line, "%d", &num);
    fclose(fp);
    return num;
}

int write_to_named_pipe(const char* pipe_name, int result_index){
    string str = "./pipe_";
    char *cstr = &str[0];
    strcat(cstr, pipe_name);
    FILE* fp = fopen(cstr, "w");
    string value = itos(result_index);
    fprintf(fp, "%s", value.c_str());
    fclose(fp);
}


int get_most_repeated(int arr[], int n) 
{ 
    sort(arr, arr + n); 
    int max_count = 1, res = arr[0], curr_count = 1; 
    for (int i = 1; i < n; i++) { 
        if (arr[i] == arr[i - 1]) 
            curr_count++; 
        else { 
            if (curr_count > max_count) { 
                max_count = curr_count; 
                res = arr[i - 1]; 
            } 
            curr_count = 1; 
        } 
    } 
  
    if (curr_count > max_count) 
    { 
        max_count = curr_count; 
        res = arr[n - 1]; 
    } 
  
    return res; 
} 

int main(int argc, char *argv[])
{
    char* validation_path = argv[1];
    char* weight_vectors_path = argv[2];
    int n = get_number_of_files_in_dir(weight_vectors_path);
    int *fd[10]; 
    for(int i =0; i < n; i++)
        fd[i] = new int[2];

    for(int i = 0; i < n; i++){
        if(pipe(fd[i]) == -1){
            cout << "error: failed to creating pipe" << endl;
            return 1;
        }
        int classifier_pid = fork();
        if (classifier_pid == 0) {
            close(fd[i][1]);
            char classifier_index[100];
            if (read(fd[i][0] ,&classifier_index ,100*sizeof(char)) <= 0) {
                cout<<"error: failed to read pipe"<<endl;
                close(fd[i][0]);
                exit(1);
            }else{
                close(fd[i][0]);
                vector<std::vector<string>> rows, weights;
                const char* ext = ".csv";
                const char* path = "/classifier_";
                int c_index = atoi(classifier_index);

                weights = readcsv(strcat(strcat(weight_vectors_path, path),
                strcat(classifier_index, ext)));
                rows = readcsv(strcat(validation_path, "/dataset.csv"));
                int row_index = 0;
                for (vector<string> row : rows)
                {
                    long double max_result_value = -1000;
                    long double result_value = -1;
                    int result_index = -1;
                    int w_index = 0;
                    for (vector<string> weight : weights)
                    {
                        int w_col = 0;
                        for (string w : weight)
                        {
                            int row_col = 0;  
                            for (string data : row)
                            {
                                if (row_col == w_col)
                                {
                                    result_value += strtol(data, w_col) * strtol(w, w_col);
                                }else if(w_col == weight.size() - 1){
                                    result_value += strtol(w, w_col);
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
                    const char* pipe_name = (itos(row_index) + "_" + itos(c_index)).c_str();
                    write_to_named_pipe(pipe_name, result_index);
                    row_index++;
                }
            }
            exit(0);
        }

        close(fd[i][0]);
        string file_index = itos(i);
        if (write(fd[i][1], file_index.c_str(), file_index.size()+1) != file_index.size()+1) {
            close(fd[i][1]);
            exit(1);
        }
        close(fd[i][1]);
    }

    for(int i = 0; i < 10; i++)
        wait(NULL);
    // cout << "linear classifiers sub-proccess are done." << endl;

    int voter_pid = fork();
    if (voter_pid == 0)
    {
        int rows_count = get_csv_rows_count(strcat(validation_path, "/dataset.csv"));
        int *labels[rows_count];
        for(int i = 0; i < rows_count; i++){
            labels[i] = new int[n];
            int j = 0;
            for (j; j < n; ++j)
            {
                const char* pipe_name = (itos(i) + "_" + itos(j)).c_str();
                int result = read_from_named_pipe(pipe_name);
                labels[i][j] = result;
            }
            int vote = get_most_repeated(labels[i], j);
            const char* pipe_name = (itos(i) + "_vote").c_str();
            write_to_named_pipe(pipe_name, vote);
        }
        exit(0);
    }
    wait(NULL);
    // cout << "voter sub-proccess is done." << endl;
    vector<std::vector<string>> label_rows;    
    label_rows = readcsv(strcat(validation_path, "/labels.csv"));
    int row_index = 0;
    int count = 0;
    for (std::vector<string> label_columns : label_rows)
    {
        for (string true_value : label_columns)
        {
            const char* pipe_name = (itos(row_index) + "_vote").c_str();
            int vote = read_from_named_pipe(pipe_name);
            if (strcmp(itos(vote).c_str(), true_value.c_str()) == 0)
                count++;
        }
        row_index++;
    }
    cout << "Accuracy: " << setprecision(4) << (((float)count * 100 )/(row_index+1)) 
    << "%" << endl;  
    return 0;
}
