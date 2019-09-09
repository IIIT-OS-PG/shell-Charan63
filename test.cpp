#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<stdio.h>
#include<termios.h>
#include<stdlib.h>
#include<deque>
#include<fstream>
#include<sys/types.h>
#include<vector>
#include<algorithm>
#include<unordered_map>
#include<dirent.h>
#include<csignal>
#include<ctime>
using namespace std;


vector<string>l_command;
vector<string>r_command;
vector<time_t>timerecord;

char prev_dir1[50];
char prev_dir2[50];

char tilde[10];
char hash1[10];

int ret_stat;

unordered_map<string,string> def1;

pid_t shellid;

struct node
{
unordered_map<char,node*>m;
bool is_end;
};node* root=NULL;


node* getnode()
{

node* temp = new node();
temp->is_end=false;
return temp;

}

void insert(string s)
{
  if(root == NULL)
  root=getnode();

  node* temp = root;

  for(int i=0;i<s.length();i++)
  {
      if(temp->m.find(s[i])!=temp->m.end())
      temp=temp->m[s[i]];
      else
      {   
          temp->m[s[i]]=getnode();
          temp=temp->m[s[i]];
      }

  }

   temp->is_end=true;
  
}

void findsuggestions(node* temp, string s,vector<string>&res)
{
   
   if(temp->is_end)
   {
       res.push_back(s);

   }  


   for(char i='a';i<='z';i++)
   {
       if(temp->m.find(i)!=temp->m.end())
       { 
           s = s+i;
           findsuggestions(temp->m[i],s,res);
       }
   }


}

pair<int,vector<string>> suggestions(string s,vector<string> &res)
{

node* temp = root;

int flag=0;

for(int i=0;i<s.length();i++)
{

if(temp->m.find(s[i])!=temp->m.end())
{
    temp=temp->m[s[i]];
}

else
{
   flag=1;
   break;
}


}


if(flag)
{
   res.clear();
   return make_pair(0,res);

}

else
{
  
   findsuggestions(temp,s,res);
   return make_pair(res.size(),res);  

}

}


void print(pair<int,vector<string>> a)
{
cout<<endl;

cout<<a.first<<endl;

for(int i=0;i<a.second.size();i++)
{
   cout<<a.second[i]<<" ";

}

cout<<endl;

}


bool issubstring(char cwd[1024],char path[1024])
{
   int m = strlen(cwd);
   int n = strlen(path);
   bool flag=false;
   int flag1 = 0;
   for(int i=0;i<=m-n;i++)
   {
     flag1 = 0;
     for(int j=0;j<n;j++)
     {
        if(cwd[i+j]!=path[j])
        { 
          flag1=1;
          break;
        }      
 
     }
      
       if(!flag1)
       {
         flag =true;
         break;
       }

   }

return flag;

}

void tildify(char cwd[1024] ,char* user)
{
    char path[1024] ;
    if(strcmp(user,"root")!=0)
    strcpy(path,"/home/");
    else
    strcpy(path,"/");
    strcat(path,user);    
  
    int m = strlen(cwd);
    int n = strlen(path);

    //cout<<endl<<path<<endl<<cwd<<endl;
    //char *p = strstr(cwd,path);

    if(issubstring(cwd,path))
    {
      memmove(cwd,cwd+n,m-n+1);
      strcpy(path,"~");
      strcat(path,cwd);
      strcpy(cwd,path);  
    }
    

}


int tokenize(char cmd[], char *tokens[],string ch)
{
  //cout<<"\nenter\n";
  char* token = strtok(cmd,ch.c_str());
  int k=0;
  while(token!=NULL)
  {
    strcpy(tokens[k++],token);
    //cout<<tokens[k-1]<<endl;
    token = strtok(NULL,ch.c_str());

  }   

  return k;

}


void callfunc_redirect(char *tokens[],int n)
{

int k = fork();
int status;
if(k==0)
{
int fd = open(tokens[n-1],O_WRONLY|O_CREAT,0666);
dup2(fd,1);
tokens[n-2] = NULL;
tokens[n-1] = NULL;
ret_stat = execvp(tokens[0],tokens);
close(fd);
}
else
waitpid(k,&status,0);
  
}

void callfunc_ip(char *tokens[],int n)
{

int k = fork();
int status;
if(k==0)
{

int fd = open(tokens[n-1],O_RDONLY,0666);
dup2(fd,STDIN_FILENO);
tokens[n-2] = NULL;
tokens[n-1] = NULL;
ret_stat = execvp(tokens[0],tokens);
close(fd);

}
else
waitpid(k,&status,0);
  
}



void callfunc_redirect_append(char *tokens[],int n)
{
int k = fork();
int status;
if(k==0)
{

int fd = open(tokens[n-1],O_WRONLY|O_CREAT|O_APPEND,0666);
dup2(fd,1);
tokens[n-2] = NULL;
tokens[n-1] = NULL;
ret_stat = execvp(tokens[0],tokens);
close(fd);

}
else
waitpid(k,&status,0);
  
}

void callfunc(char *tokens[],int n)
{
int k = fork();
int status;


if(k==0)
{

if(strcmp(tokens[0],"echo")==0&&tokens[1][0]=='$')
{

   if(tokens[1][1]=='$')
   {
       string f = to_string(shellid);
       const char *v =f.c_str();
       char temp[10];
       strcpy(temp,v);
       strcpy(tokens[1],temp);
       tokens[2]=NULL;
   }
   else if(tokens[1][1]=='?')
   {
       string f = to_string(ret_stat);
       const char *v =f.c_str();
       char temp[10];
       strcpy(temp,v);
       strcpy(tokens[1],temp);
       tokens[2]=NULL;

   }
   else
   {
   
   char temp[10];
   strncpy(temp,tokens[1]+1,strlen(tokens[1])-1);
   strcpy(tokens[1],getenv(temp));
   tokens[2]=NULL;

   }

}


string s(tokens[0],tokens[0]+strlen(tokens[0]));

//cout<<"debug"<<s<<endl;

char buf[1024];

char * alias_tok[10];

for(int i=0;i<10;i++)
alias_tok[i] = new char[50]();

char * total[20];
for(int i=0;i<20;i++)
total[i] = new char[50]();


if(find(l_command.begin(),l_command.end(),s)!=l_command.end())
{
   
    //cout<<"hello\n";
    strcpy(buf,r_command[find(l_command.begin(),l_command.end(),s)-l_command.begin()].c_str());
    //cout<<buf<<endl;
    int x = tokenize(buf,alias_tok," ");

    //cout<<endl<<x<<endl;

    for(int p=0;p<x;p++)
    {
        strcpy(total[p],alias_tok[p]);
        //cout<<endl<<total[p]<<endl;
    }

   if(n==1)
   total[x]=NULL;

  
   if(n>=2)
   {
 
    for(int p=1;p<n;p++)
    {
        strcpy(total[x+p-1],tokens[p]);
        //cout<<endl<<total[x+p-1]<<endl;
  
    }

    total[x+n-1]=NULL;

  }

   //cout<<endl<<total[0]<<endl<<total[1]<<endl<<total[2]<<endl;

    ret_stat = execvp(total[0],total);

}

else
{
ret_stat = execvp(tokens[0],tokens);
}

}

else
waitpid(k,&status,0);
  
}


void callfunc_background(char *tokens[],int n)
{
int k = fork();
int status;

if(k==0)
{
setpgid(0,0);
/*
By default chlid will have same process group id than parent

as we know atmost 1 process group can be in foreground at a time

if we change the group id of child ( above gives group no same as process no )

it is thrown to background 

*/
tokens[n-1]=NULL;
int fd = open("/dev/null",O_WRONLY|O_CREAT|O_APPEND,0666);
dup2(fd,1);
ret_stat = execvp(tokens[0],tokens);

}

else
{
cout<<"process id : "<<k<<endl;
}
  
}



void add(string path)
{

DIR *d ;
struct dirent *k = new dirent();

d=opendir(path.c_str());

while((k=readdir(d))!=NULL)
{
  
    insert(k->d_name);
    //cout<<k->d_name<<endl;


}

closedir(d);

}


void init()
{


ifstream in;
in.open(".myrc",ios::in);
char buff[1024];
char pat[]="PATH";
int flag=0;
while(!in.eof())
{
  in.getline(buff,1024);
  if(strstr(buff,pat))
  {
      flag=1;
      break;
  }


}

in.close();

if(flag)
{
char* path_tokens[4];
path_tokens[0]=new char [5];
path_tokens[1]=new char [2];
path_tokens[2]=new char [1024];
path_tokens[3]=new char [2];

int tok = tokenize(buff,path_tokens," ");
setenv(pat,path_tokens[2],1);

}

else
{

in.open("/etc/environment",ios::in);
while(!in.eof())
{
  in.getline(buff,1024);
  if(strstr(buff,pat))
  {
      flag=1;
      break;
  }


}

in.close();


char path_cp[1024];
strncpy(path_cp,buff+6,strlen(buff)-7);
setenv(pat,path_cp,1);

}

int usr = getuid();
in.open("/etc/passwd",ios::in);

char * tokens[7];

for(int i=0;i<7;i++)
tokens[i]=new char[50];

while(!in.eof())
{
  in.getline(buff,1024);

  int te = tokenize(buff,tokens,":");

  if(atoi(tokens[2])==usr)
  break;
  
}

in.close();

setenv("USER",tokens[0],1);
setenv("HOME",tokens[5],1);


in.open("/etc/hostname");

char buf[300];

while(!in.eof())
{

in>>buf;

}

setenv("HOST",buf,1);


//cout<<endl<<buf<<endl;

in.close();





//setting ps1

ifstream in1;

in1.open(".myrc",ios::in);
char f[1024];
char ps[4]="PS1";

char * tp[4];

for(int i=0;i<4;i++)
{
tp[i] = new char[10];
}

int ax = 0;

while(!in1.eof())
{
  in1.getline(f,1024);
  if(strstr(f,ps))
  {

      int t = tokenize(f,tp," ");

      strcpy(tilde,tp[2]);
      strcpy(hash1,tp[3]);  
      ax=1;
 
      break;       
   }


}

if(!ax)
{

      strcpy(tilde,"$");
      strcpy(hash1,"#");


}


string g1(tilde,tilde+strlen(tilde));
string g2(hash1,hash1+strlen(hash1));
string g = g1+" "+g2;

setenv("PS1",g.c_str(),1);

in1.close();



in.open(".myrc",ios::in);

char *alias_tokens[2];

for(int i=0;i<2;i++)
alias_tokens[i]=new char [50];

char* tep [2];

for(int i=0;i<2;i++)
tep[i]=new char [50];

while(!in.eof())
{
   in.getline(buf,300);
   if(strstr(buf,"alias"))
   {

   //cout<<buf<<endl;
   int te = tokenize(buf,alias_tokens,"=");
   int se = tokenize(alias_tokens[0],tep," ");
   strcpy(alias_tokens[0],tep[1]);
   string s1(alias_tokens[0],alias_tokens[0]+strlen(alias_tokens[0]));
   string s2(alias_tokens[1],alias_tokens[1]+strlen(alias_tokens[1]));
   //cout<<s1<<endl<<s2<<endl;
   l_command.insert(l_command.begin(),s1);
   r_command.insert(r_command.begin(),s2);
   }
 
}

in.close();
//Adding all files to trie;
///home/charan/bin:/home/charan/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin

add("/bin/");
add("/sbin/");
add("/usr/local/bin/");
add("/usr/local/sbin/");
add("/usr/sbin/");
add("/usr/bin/");

//open

in.open(".myrc",ios::in);

char b[1024];

char *to1[5];

for(int i=0;i<5;i++)
{
   to1[i]=new char[20];

}

while(!in.eof())
{
  in.getline(b,1024);
  if(strstr(b,"open"))
  {
      int t = tokenize(b,to1," ");
  
      
      for(int i=1;i<t-1;i++)
      {
          
          // cout<<"\nhello\n";
           string sx(to1[i],to1[i]+strlen(to1[i]));
           string sy(to1[t-1],to1[t-1]+strlen(to1[t-1]));           
           def1[sx]=sy;

//            cout<<endl<<sx<<endl;
//            cout<<endl<<sy<<endl;

      }
       
  }


}

in.close();


//FOR NOT INVOKED ALARM

in.open(".alarm",ios::in);
char timebuff[100];
while(!in.eof())
{
  
  in.getline(timebuff,100);
  time_t x = atoi(timebuff);
  time_t curr = time(NULL);
  if(curr>x)
  {
      if(x!=0)
      cout<<"Missed alarm : "<<ctime(&x);

  }
  
}

in.close();

//clear ALARM FILE

ofstream out;
out.open(".alarm",ios::out|ios::trunc);
out.close();



}



void cd(char *tokens[],char user[],int n)
{

int k = fork();
int status;
char temp[30];

if(k==0)
{
  
  if(n==1||strcmp(tokens[n-1],"~")==0)  
  {
     strcpy(temp,"/home/");
     strcat(temp,user);
     strcpy(prev_dir2,prev_dir1);
     strcpy(prev_dir1,temp);
     
     chdir(temp);

     
  }
  else if(n>1&&strcmp(tokens[n-1],"-")==0)
  {
     chdir(prev_dir2);

  }
  else if(n>1)
  {
    strcpy(prev_dir2,prev_dir1);
    strcpy(prev_dir1,tokens[n-1]);
    chdir(tokens[n-1]);
  }

}
else
{
waitpid(k,&status,0);
}

}
void handle(int sign)
{

time_t x = time(NULL);
timerecord.erase(find(timerecord.begin(),timerecord.end(),x));

cout<<"\nALARM : INVOKED at "<<ctime(&x)<<endl;

//exit(0);      

}

void alarm1(char *tokens[],int n)
{

//int k = fork();
int status;
//if(k==0)
//{

int len = strlen(tokens[n-1]);
int t_sec = atoi(tokens[n-1]);

   alarm(t_sec);
   signal(SIGALRM,handle);
   time_t x =time(NULL);
   time_t tot = t_sec+x;
   timerecord.push_back(tot);
   cout<<"ALARM : set at "<<ctime(&x);

//}
//else
//{
  // waitpid(k,&status,0);
   //signal(SIGCHLD,SIG_IGN);
//}

}



void open(char *tokens[],int n)
{

char *buff[2];
for(int i=0;i<2;i++)
buff[i] = new char[10];

int k = fork();
int status;
if(k==0)
{

char te[10];

strcpy(te,tokens[n-1]);

int temp = tokenize(te,buff,".");

if(def1.find(buff[1])!=def1.end())
{
  strcpy(tokens[0],def1[buff[1]].c_str());

}

for(int i=0;i<n;i++)
{
  cout<<endl<<tokens[i]<<endl;

}

ret_stat = execvp(tokens[0],tokens);

}
else
waitpid(k,&status,0);
  
}

void alias(char *tokens[],int n)
{

string s1(tokens[1],tokens[1]+strlen(tokens[1]));

string s2(tokens[3],tokens[3]+strlen(tokens[3]));

for(int i=4;i<n;i++)
{
   string s(tokens[i],tokens[i]+strlen(tokens[i]));
   s2=s2+" "+s; 
 
}

//cout<<endl<<s1<<endl<<s2<<endl;

   l_command.insert(l_command.begin(),s1);
   r_command.insert(r_command.begin(),s2);  
}

void exithandler(int sign)
{
  
}

void exit1()
{

   ofstream out;
   out.open(".alarm",ios::app);
   for(int i=0;i<timerecord.size();i++)
   {
       out<<timerecord[i]<<endl;   

   }
   out.close();
   
   _Exit(0);

   //raise(SIGTSTP);
}

int main()
{
    init();
    
    cout<<"\n";
    cout<<"\n";

    int donothing=0;
  
   cout<<"-----------------------------------------shell starts here-------------------------------------------------------------------\n\n\n";
   char * user;
   char host[1024];
   char cwd[1024];

   deque <string> history;
   history.push_back(""); 

   int xx=1;

   signal(SIGINT,exithandler);

   shellid = getpid();
  
   while(1)
     {

      donothing=0;
      user =  getenv("USER");
      gethostname(host,1024);
      getcwd(cwd,1024);
      tildify(cwd,user);
      if(strcmp(user,"root")==0)
      cout<<user<<"@"<<host<<":"<<cwd<<hash1<<" ";
      else
      cout<<user<<"@"<<host<<":"<<cwd<<tilde<<" ";

      char cm[1024];
      int z;
  
     // cin.getline(cm,1024);
    

   
    static struct termios prev1, tempp;
    tcgetattr(STDIN_FILENO, &prev1);
    tempp = prev1;
    tempp.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tempp);
      
    //cin.getline(cm,1024);
   // fflush(stdout);
     char c;
     int len =0;
     z=0;
     int hist_ind=0;
     int prev=0;
     int fl=0;
  
     while (1) {

         if(fl)
         {
           fl=0;
         }
         else
         c=getchar();
         
         if (c == 0x7f) 
         {
            
           if(z>0)
           {
            cout<<"\b \b";
            //\b just moves cursor back it don't actually delete
            z--;
            len--;
            prev = len;
            }
         }
         else if(c=='\t')
         {
             
            vector<string>res;
           
            string ser(cm,cm+len);
           
            pair<int,vector<string>> a = suggestions(ser,res);

            if(a.first==1)
            { 
           
             for(int i=0;i<len;i++)
             {
                cout<<"\b \b";
             }

             //cout<<a.second[0];
           
             strcpy(cm,a.second[0].c_str());
             cout<<cm;
             z=strlen(cm);
             len=z;
             prev = strlen(cm);

            }

            else
            {

                c = getchar();
               
                if(c=='\t')
                {
                    print(a);
                    //strcpy(cm,"");
                    //len=0;
                    donothing=1;
                    z=0;
                    len=0;
                    break;

                }

                else
                {
                   fl=1;
                }
              


            }


         }
         else if(c==(char)27)
          { 

            char tempx = getchar();
            char tempy = getchar();
            if(tempy=='D')
            {
              if(z>0)
              {//left
                z--;
                printf("\033[1D");
              }

            }
            else if(tempy=='C')
            {//right
               z++;
               printf("\033[1C");
            }
            else if(tempy=='A')
            {//top
              
              for(int i=0;i<prev;i++)
              {
                cout<<"\b \b";
              }
        
              hist_ind = (hist_ind -1 + xx)%xx ;
              //cout<<"index inside top"<<hist_ind<<endl;
             
              z = history[hist_ind].length();
              len = z;
              //cout<<"len "<<len<<endl;
             // cout<<endl<<history[hist_ind]<<endl;
              //printf("%s",history[hist_ind]);
             
              strcpy(cm,history[hist_ind].c_str());
              cout<<cm;
              prev = strlen(cm);
              
            }

            else if(tempy=='B')
            {//bottom
              for(int i=0;i<prev;i++)
              {
                cout<<"\b \b";
              }
              hist_ind = (hist_ind + 1)%xx ;
              z = history[hist_ind].length();
              len = z;
              //printf("%s",history[hist_ind]);
              strcpy(cm,history[hist_ind].c_str());
              cout<<cm;
              prev = strlen(cm);
              
            }
        }
        else if (c == '\n') { break;}
        else
        {
          cout<<c;
          len++;
          cm[z++]=c;
          prev = len;
        
        }

            
     }  
     
     tcsetattr(0, TCSANOW, &prev1);
    
if(!donothing)
{
      cm[len]='\0';
      string str(cm, cm + len);
      history.push_back(str);
      xx++;
      //cout<<endl<<history[xx++];
      hist_ind = xx;
    //  cout<<endl<<xx;
  
    
      //cout<<endl<<cm<<endl;

      //cout<<endl;
  
       string tile("~");
       int pox = str.find(tile);
    
       if(pox>=0&&pox<=str.length())
       {
       string te_us(user,user+strlen(user));
       string pa = "/home/"+te_us;
       str.replace(pox,1,pa);
       
       const char * st_tem=str.c_str();
       strcpy(cm,st_tem);
       }
  
      char* cmd[10];
   
      for(int i=0;i<5;i++)
      cmd[i]=new char[100];
  
      int commands = tokenize(cm,cmd,"|");
      
      /*cout<<endl<<commands<<endl;
    
        
       for(int i=0;i<commands;i++)
        {
          cout<<cmd[i]<<" "<<i;         
 
        }*/

   // cout<<endl;

//cout<<endl<<i<<" : wth happening here\n";

cout<<endl;
    
if(commands>1)
{ 
 
       int temp = fork();
       int stat;

       int temp1;
       int pfd[2];
       int k; 
 
  if(temp==0)
  {
       for(int j=0;j<commands-1;j++)
       {   
    
             char* tokens[10];
             for(int p=0;p<10;p++)
             {
               tokens[p]=new char[100];
              
             }

             int tok = tokenize(cmd[j],tokens," ");
             
             tokens[tok]=NULL;

             if(pipe(pfd)<0)cout<<"\npipe error\n";  
 
             k = fork();
             int status;
             
             if(k==0)
             {
                dup2(pfd[1],1);
                ret_stat = execvp(tokens[0],tokens);
                close(temp1);
               
             }
             else if(k>0)
             {
                waitpid(k,&status,0);
                close(pfd[1]);
                dup2(pfd[0],0);
                temp1 = pfd[0];
                
                
             }
             
            

           
        }//for close

       close(pfd[0]);

           if(k>0)
           {

             char* last[10];
             for(int p=0;p<10;p++)
             {
              last[p]=new char[50];
              
             }

             int in = tokenize(cmd[commands-1],last," ");
             
             last[in]=NULL;

             if(in>1&&strcmp(last[in-2],">")==0)
             {
                 int fd = open(last[in-1],O_WRONLY|O_CREAT,0666);
                 dup2(fd,1);
                 last[in-2] = NULL;
                 last[in-1] = NULL;
                 ret_stat = execvp(last[0],last);
                 close(fd);
             }
             else if(in>1&&strcmp(last[in-2],">>")==0)
             {
                  int fd = open(last[in-1],O_WRONLY|O_CREAT|O_APPEND,0666);
                  dup2(fd,1);
                  last[in-2] = NULL;
                  last[in-1] = NULL;
                  ret_stat = execvp(last[0],last);
                  close(fd);
             }
             else
             {
                  ret_stat = execvp(last[0],last);

             }

           }

 //execvp(last[0],last);

        //}

}
else
waitpid(temp,&stat,0);
      

}//if - close

else if(commands==1)
{

       char* tokens[10];

       for(int j=0;j<10;j++)
       tokens[j]=new char[100];

       int tok = tokenize(cm,tokens," ");
      
       tokens[tok]=NULL;
      

     if(tok>1&&strcmp(tokens[tok-2],">")==0)
      { 
         callfunc_redirect(tokens,tok);
      }
      else if(tok>1&&strcmp(tokens[tok-2],">>")==0)
      {
         callfunc_redirect_append(tokens,tok);
        
      }
      else if(tok>1&&strcmp(tokens[tok-2],"<")==0)
      {
         callfunc_ip(tokens,tok);
 
      }
      else if(tok>1&&strcmp(tokens[tok-1],"&")==0)
      {
           if(strcmp(tokens[0],"cd")!=0)
           {
             callfunc_background(tokens,tok);
           }
           

      }
      else
      {
 
        if(strcmp(tokens[0],"cd")==0)
        {
             cd(tokens,user,tok);
        }
        else if(strcmp(tokens[0],"fg")==0)
        {
              //fg(tokens);

        }
        else if(strcmp(tokens[0],"bg")==0)
        {
             //bg(tokens);

        }
        else if(strcmp(tokens[0],"alias")==0)
        {
             alias(tokens,tok);

        }
        else if(strcmp(tokens[0],"open")==0)
        {
             
             //for(int i=0;i<tok;i++)
             //cout<<tokens[i]<<" ";
             open(tokens,tok);

        }
        else if(strcmp(tokens[0],"alarm")==0)
        {
 
              alarm1(tokens,tok);

        }
        else if(strcmp(tokens[0],"exit")==0)
        {
               exit1();
           
        }
        else
        callfunc(tokens,tok);
      }  
      
}//else

}

 }//while1

}//main
    



