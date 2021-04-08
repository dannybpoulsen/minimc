# Location CPA  
The location CPA is the simplest CPA imaginable. 
It only tracks the program location of each computational thread of
the program. 
For single-threaded program this means we essentially  the structure of
the control-flow-automaton.  


It it rarely useful to perform a reachability analysis on a location CPA. 
This CPA is mostly useful because it can be combined with others
CPAs. Thus making it unncessary for those CPAs to track the program
location.  

## Example  

!!! todo
	Insert an example showing a pgraph subcommand run on a small example.
