# AntCloud
 - this project includes server, worker and client code

# Description
- AntCloud is a distributed processing application that enables users to execute their code on a remote server with high-performance computing resources.
- AntCloud is designed for users who have limited processing power on their local PC and want to leverage the benefits of cloud computing and distributed processing without incurring high costs.
- By using AntCloud, users can easily upload their code (along with arguments if needed), the main server will chose
a suitable workstation, the workstations will compile and execute it sending back the output generated.
- AntCloud is a convenient and affordable solution for users who need to run complex and intensive code on a regular basis.

# Instalation
 - make sure you have gcc on your computer
 - clone the project on local
 - go to the project directory and run make in terminal for the setup

# Notes
 - 

# Usage 
 - run ./server in a terminal to launch the server
 - run ./worker in one or more terminals to launch workers
 - run ./client in one or more terminals to launch workers
 - the client will start with a CLI to help the user
### Client 
      root:AntCloud/build$ ./client 
      Connection to server succesful!
                  ###########################
                             100%
                             
                  ****Welcome to AntCloud****
  
      >

- the CLI will come with a help menu
 
      >help
  
      ANTCLOUD(1)
      
      NAME
             antcloud - distributed processing application
      
      SYNOPSIS
             ./antcloud
      
      DESCRIPTION
             Sends a source file to a server that compiles and executes with the help of workstations.
      
      COMMANDS
             ? help -h 
                     Output a usage message and exit.
             run
                     Upload a source file to the server.
                     USAGE 
                             run [SOURCE FILE] [ARG1] [ARG2] ...
             exit q
                     Exit the program.

### Server
- the server show all the new connections and their type

      root:/AntCloud/build$ ./server 
      Binding successful on socket: 3!
      Connection accepted: 5
      Added new client: 5
      Connection accepted: 6
      Added new worker: 6
      Linking client: successful to host cd: 6!
      Connection accepted: 7
      Added new worker: 7
      Connection accepted: 8
      Added new client: 8
      Linking client: successful to host cd: 7!

# Credits
 - base64 library (https://github.com/elzoughby/Base64/tree/master)
