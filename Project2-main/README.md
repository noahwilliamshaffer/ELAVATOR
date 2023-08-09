# Project2
Project two for FSU COP4610 
Matthew Kolnicki, Jalal Jean-Charles, Noah Shaffer

Contents of tar archive:

  README
  
  gitlog
    Contains png's of github log

  Part1:
    Contains all of the relevant files needed for the implementation of part 1 of the project
    
    
    empty.c
        empty file for comparison with syscalls in part1.c
        
    part1.c
        four syscalls.
        
    empty.trace
        output of strace on the empty.c executable
        
    part1.trace
        output of strace on the part1.c executable
    
  Part2:
    Contains all of the relevant files needed for the implementation of part 2 of the project (creating a module named my_timer). 
    
    
    my_timer.c
        My_timer implementation. Compares the time between current kernel time and the time at the most recent call to proc.
        
    makefile
    
    Makefile Description:
    creates a module named my_timer based upon the lixux version stored at /lib/modules/`uname -r`/build. 
    Calling make creates my_timer.ko which can be used to load in the module. Default creates the necessary object and kernel files to load in the module.
    Make clean removes all object and kernel files.
    
    how to compile:
    sudo make
    sudo insmod my_timer.ko
    
    remove module with rmmod my_timer
      
  
  Part3:
    Contains all of the relevant files needed for the implementation of part 3 of the project (creating and running the elevator module).
  
    elevator.c
        This is the main function that runs the elevator. The elevator will remain in iether up or down status until reaching the end of the 10 level 
        building, unless it reaches a load. The elevator has a status that starts at offline, switches to idle, defualts to up. The status switches 
        to load when loading or unloading passengers on the elevator, and then swithes the status back to the the direction that it was going before. 
        
    start_elevator.c
        This switches the elevator from offline to idle and initiates the normal processing of the elevator.
    stop_elevator.c
        This stops the elevator from taking on more passengers and the elevator drops off the remaining passengers. When the elevator has no 
        more passengers the elevator switches it's current status to offline. 
        
    issue_request.c
        This creates random animals to be put into queue on a random floor of the building to be processed by the elevator. 
          
    syscalls.h
        This just declares the syscalls.
    

    Makefile Description:
        created three new system calls which can be used inside the newly created module elevator. start_elevator creates a thread for the elevator 
        to begin processing requests created by the system call issue requests. Stop_elevator kill the thread created by start_elevator. 
        Default creates the necessary object and kernel files to load in the module. Make clean removes all object and kernel files.
    
    how to compile:
      sudo make
      sudo insmod elevator.ko

      remove module with rmmod elevator
      
  

Division of Labor:
  All group memeber contributed to each part of the project. The entire project was done in a zoom call where all participants were present.
  
  Known Bugs:
    No known bugs
  
  Special considerations:
    None

