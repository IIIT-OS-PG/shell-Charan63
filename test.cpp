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
using namespace std;


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


int tokenize(char cmd[], char *tokens[])
{
  //cout<<"\nenter\n";
  char* token = strtok(cmd," ");
  int k=0;
  while(token!=NULL)
  {
    strcpy(tokens[k++],token);
    //cout<<tokens[k-1]<<endl;
    token = strtok(NULL," ");

  }   

  return k;

}

int pipify(char cm[], char *cmd[])
{
  //cout<<"\nenter\n";
  char* token = strtok(cm,"|");
  int k=0;
  while(token!=NULL)
  {
    strcpy(cmd[k++],token);
    //cout<<tokens[k-1]<<endl;
    token = strtok(NULL,"|");

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
execvp(tokens[0],tokens);
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
execvp(tokens[0],tokens);
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
execvp(tokens[0],tokens);
close(fd);

}
else
waitpid(k,&status,0);
  
}

void callfunc(char *tokens[])
{
int k = fork();
int status;

if(k==0)
execvp(tokens[0],tokens);

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
tokens[n-1]=NULL;
int fd = open("/dev/null",O_WRONLY|O_CREAT|O_APPEND,0666);
dup2(fd,1);
execvp(tokens[0],tokens);

}

else
{
cout<<"process id : "<<k<<endl;
}
  
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
     chdir(temp);
  }
  else if(n>1)
  {
    chdir(tokens[n-1]);
  }

}
else
waitpid(k,&status,0);


}


int main()
{

    cout<<"\n";
    cout<<"\n";
  
   cout<<"-----------------------------------------shell starts here-------------------------------------------------------------------\n\n\n";
   char * user;
   char host[1024];
   char cwd[1024];

   deque <string> history;
   history.push_back(""); 

   int xx=1;
  
   while(1)
     {
      user =  getenv("USER");
      gethostname(host,1024);
      getcwd(cwd,1024);
      tildify(cwd,user);
      if(strcmp(user,"root")==0)
      cout<<user<<"@"<<host<<":"<<cwd<<"# ";
      else
      cout<<user<<"@"<<host<<":"<<cwd<<"$ ";

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
     while (1) {
         c=getchar();
         
         if (c == 0x7f) 
         {
            
           if(z>0)
           {
            cout<<"\b \b";
            z--;
            len--;
            prev = len;
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
    
      cm[len]='\0';
      string str(cm, cm + len);
      history.push_back(str);
      xx++;
      //cout<<endl<<history[xx++];
      hist_ind = xx;
    //  cout<<endl<<xx;
  
    
      //cout<<endl<<cm<<endl;

      //cout<<endl;
      char* cmd[10];
   
      for(int i=0;i<5;i++)
      cmd[i]=new char[100];
  
      int commands = pipify(cm,cmd);
      
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

             int tok = tokenize(cmd[j],tokens);
             
             tokens[tok]=NULL;

             if(pipe(pfd)<0)cout<<"\npipe error\n";  
 
             k = fork();
             int status;
             
             if(k==0)
             {
                dup2(pfd[1],1);
                execvp(tokens[0],tokens);
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

             int in = tokenize(cmd[commands-1],last);
             
             last[in]=NULL;

             if(in>1&&strcmp(last[in-2],">")==0)
             {
                 int fd = open(last[in-1],O_WRONLY|O_CREAT,0666);
                 dup2(fd,1);
                 last[in-2] = NULL;
                 last[in-1] = NULL;
                 execvp(last[0],last);
                 close(fd);
             }
             else if(in>1&&strcmp(last[in-2],">>")==0)
             {
                  int fd = open(last[in-1],O_WRONLY|O_CREAT|O_APPEND,0666);
                  dup2(fd,1);
                  last[in-2] = NULL;
                  last[in-1] = NULL;
                  execvp(last[0],last);
                  close(fd);
             }
             else
             {
                  execvp(last[0],last);

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

       int tok = tokenize(cm,tokens);
      
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
             fg(tokens);

        }
        else if(strcmp(tokens[0],"bg")==0)
        {
             bg(tokens);

        }
        else
        callfunc(tokens);
      }  
      
}//else

 }//while1

}//main
    



