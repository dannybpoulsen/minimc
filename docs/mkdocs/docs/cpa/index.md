# Configurable Program Analysis  
{{minimc}} is build around the configurable program analysis framework
pioneered by
[CPAChecker](https://cpachecker.sosy-lab.org/doc.php). Internally
{{minimc}} thus have different state representations and behaviours on
how states are merged  and  cover each other during exploration
. 
Together the state representation and these operations form what we
can call CPA.
This allow reusing many
components (e.g. reachability algorithms) in the core of {{minimc}}
while easily extending the analytical power of it by "just" adding an
extra CPA that tracks the information we are interested in. 

!!! note
	The analysis implemented in {{minimc}} will select which CPA  to use
	thus for the most parts users do not need to worry about CPAs.  

!!! todo
	Add  an architecture picture
