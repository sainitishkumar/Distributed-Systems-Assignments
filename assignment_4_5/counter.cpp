#include <iostream>
#include <chrono>
#include <ctime>
#include <thread>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <stdlib.h>

using namespace std;

void exec_thread(int id)
{
    auto now_time = chrono::system_clock::now();
    auto sleep_time = chrono::system_clock::to_time_t(now_time);
    tm mylocal_time = *localtime(&sleep_time);

    cout<<"thread "<<id<<" sleep time ";
    cout<<mylocal_time.tm_hour<<"::"
        <<mylocal_time.tm_min<<"::"
        <<mylocal_time.tm_sec<<"\n";

    std::this_thread::sleep_for(std::chrono::seconds(3));
}

// global shared variable between threads
int global_total = 0, global_value=0;
std::mutex total_lock;

void inc_global(int thread_id, int value)
{
    // lock the scope of this function
    std::lock_guard<std::mutex> lock(total_lock);
    total_lock.lock();

    //increment the global shared variable
    global_total+=value;

    total_lock.unlock();
}

void read_file(vector<int> file_contents, 
                int random_nums, int thread_count, 
                int id, string fname)
{
    int sum1 = 0;
    //calc the sum for numbers assigned to threads
    if(id!=thread_count-1)
    {
        int each = random_nums/thread_count;
        for(int i=id*each;i<id*each+each;i++)
        {
            sum1+=file_contents[i];
        }
    }
    else
    {
        int each = random_nums/thread_count;
        for(int i=id*each;i<random_nums;i++)
        {
            sum1+=file_contents[i];
        }
    }
    total_lock.lock();
    //increment the global shared variable by sum1
    global_total+=sum1;
    total_lock.unlock();
}

void read_files(vector<string> file_names, int m_thread_count, int id)
{
    int sum1=0;
    int each = file_names.size()/m_thread_count;
    if(id!=m_thread_count-1)
    {
        for(int i=id*each;i<id*each+each;i++)
        {
            total_lock.lock();
            // cout<<file_names[i]<<" "<<id<<endl;
            total_lock.unlock();
            ifstream f1;
            f1.open(file_names[i]);
            int x1 = 0;
            while(f1>>x1)
                sum1+=x1;
        }
    }
    else
    {
        for(int i=id*each;i<file_names.size();i++)
        {
            ifstream f1;
            f1.open(file_names[i]);
            int x1 = 0;
            while(f1>>x1)
                sum1+=x1;
        }
    }
    total_lock.lock();
    //increment the global shared variable by sum1
    global_total+=sum1;
    total_lock.unlock();
}

int main(int argc, char const *argv[])
{
    /*
    *   Generate a and b files and populate
    *   with random numbers and keep the count
    *   Once finished, get out and fork()
    *   Then launch threads for both the processes
    *   and count. Finally the parent prints result.
    */
    int fd[2];
    pipe(fd);
    srand(102);
    if(argc!=6){
        cout<<"Not enough arguments";
        return 1;
    }
    int a_count = atoi(argv[1]);
    int b_count = atoi(argv[2]);
    int random_nums = atoi(argv[3]);
    int m_thread_count = atoi(argv[4]);
    int c_thread_count = atoi(argv[5]);

    // fstream* a_files = new fstream[a_count];
    // fstream* b_files = new fstream[b_count];

    vector<FILE*> a_files;
    vector<FILE*> b_files;

    for(int i=0;i<a_count;i++)
    {
        FILE* temp = new FILE;
        a_files.push_back(temp);
    }

    for(int i=0;i<b_count;i++)
    {
        FILE* temp = new FILE;
        b_files.push_back(temp);
    }
    
    int total = 0;
    // track file number
    int count=0;
    for(auto i :a_files)
    {
        string fname = "a_";
        fname.append(to_string(count));
        i = fopen(fname.c_str(), "w+");
        if(i!=NULL)
        {
            for(int j=0;j<random_nums;j++)
            {
                int temp = rand()%101;
                total+=temp;
                string t1 = to_string(temp).append("\n");
                fprintf(i, t1.c_str());
            }
        }
        fclose(i);
        count++;
    }
    count=0;
    for(auto i :b_files)
    {
        string fname = "b_";
        fname.append(to_string(count));
        i = fopen(fname.c_str(), "w+");
        if(i!=NULL)
        {
            for(int j=0;j<random_nums;j++)
            {
                int temp = rand()%101;
                total+=temp;
                string t1 = to_string(temp).append("\n");
                fprintf(i, t1.c_str());
            }
        }
        fclose(i);
        count++;
    }

    pid_t pid = fork();
    
    a_files.clear();
    b_files.clear();

    if(pid!=0) // parent
    {
        cout<<"\nCalculated while creating: "<<total<<endl;
        // std::thread *m_threads = new thread[m_thread_count];
        // std::thread m_threads[m_thread_count];
        vector<std::thread> m_threads;
        if(a_count < m_thread_count)
        {
            // one file multiple threads
            int each = m_thread_count/a_count;
            int remaining = m_thread_count%a_count;
            
            int tcount = 0;
            count=0;
            for(int i=0;i<a_count; i++)
            {
                string fname = "a_";
                fname.append(to_string(i));
                int thread_count = (remaining>0) ? (each+1) : each;
                remaining--;
                vector<int> file_contents;

                ifstream file_ptr;
                file_ptr.open(fname);
                int x;
                while(file_ptr>>x)
                {
                    file_contents.push_back(x);
                }
                for(int j=0;j<thread_count;j++)
                {
                    m_threads.push_back(
                        std::thread(read_file, file_contents, random_nums, 
                        thread_count, j, fname));
                }
            }
            for(int j=0;j<m_threads.size();j++)
            {
                m_threads[j].join();
            }
            // cout<<"Total sum of a files: "<<global_total<<endl;
        }
        else
        {
            // one thread multiple files
            int each = a_count/m_thread_count;
            vector<string> file_names;
            for(int i=0;i<a_count;i++)
            {
                string s="a_";
                file_names.push_back(s.append(to_string(i)));
            }
            for(int i=0;i<m_thread_count;i++)
            {
                m_threads.push_back(
                    std::thread(read_files, file_names, m_thread_count, i)
                );
            }
            for(int i=0;i<m_thread_count;i++)
            {
                m_threads[i].join();
            }
            // cout<<"Total sum of b files: "<<global_total<<endl;
        }
        wait(NULL); 
  
        // closing the standard input  
        close(0); 
    
        // no need to use the write end of pipe here so close it 
        close(fd[1]);  
    
        // duplicating fd[0] with standard input 0 
        dup(fd[0]);  
        int arr[1]; 
    
        // n stores the total bytes read succesfully 
        int n = read(fd[0], arr, sizeof(arr));
        global_total+=arr[0];
        cout<<"Total sum of files from threads: "<<global_total<<endl;
    }
    else
    {
        // std::thread *c_threads = new thread[c_thread_count];
        vector<std::thread> c_threads;
        if(b_count < c_thread_count)
        {
            // one file multiple threads
            int each = c_thread_count/b_count;
            int remaining = c_thread_count%b_count;
            
            int tcount = 0;
            count=0;
            for(int i=0;i<b_count; i++)
            {
                string fname = "b_";
                fname.append(to_string(i));
                int thread_count = (remaining>0) ? (each+1) : each;
                remaining--;
                vector<int> file_contents;

                ifstream file_ptr;
                file_ptr.open(fname);
                int x;
                while(file_ptr>>x)
                {
                    file_contents.push_back(x);
                }
                for(int j=0;j<thread_count;j++)
                {
                    c_threads.push_back(
                        std::thread(read_file, file_contents, random_nums, 
                        thread_count, j, fname));
                }
            }
            for(int j=0;j<c_threads.size();j++)
            {
                c_threads[j].join();
            }
            // cout<<"Total sum of a files: "<<global_total<<endl;
            
        }
        else
        {
            // one thread multiple files 
            int each = b_count/c_thread_count;
            vector<string> file_names;
            for(int i=0;i<b_count;i++)
            {
                string s="b_";
                file_names.push_back(s.append(to_string(i)));
            }
            for(int i=0;i<c_thread_count;i++)
            {
                c_threads.push_back(
                    std::thread(read_files, file_names, c_thread_count, i)
                );
            }
            for(int i=0;i<c_thread_count;i++)
            {
                c_threads[i].join();
            }
            // cout<<"Total sum of a files: "<<global_total<<endl;
            
        }
        close(fd[0]);  
        int arr[] = {global_total};
       // closing the standard output 
        close(1);     
    
        // duplicating fd[0] with standard output 1 
        dup(fd[1]);   
        write(1, arr, sizeof(arr));
    }
    // cout<<"thread 1\n";
    // std::thread t1 (exec_thread, 1);
    // t1.join();
    // std::thread t2 (exec_thread, 2);
    // t2.join();

    return 0;
}