# The Welfare Crook problem by Prof. D. Gries

The big idea behind my solution is to have each process pass on all the matches
that it found to the next process and the last process will collect and broadcast 
the results to everybody.

This makes the solution extendable very easily to more than 3 arrays.    
Those extensions would also have quite good performance, as each array and process goes
through it's array exactly once.

## Implementation

My solution includes three different programs depending on your rank.
- If you are the first one, just send
- If you are in the middle, recieve, filter and send ones that fit
- If you are the last one, receive and write down the results

### First
The first process is the one causing the most complexity, because it will send it's
 whole array one by one, after that it will send a sentinel value (-1) to tell the 
 next process it is done, and will continue onto the joint section   

### Middle
The middle ones will receive  a value from the previous process, ignore any values 
in its' array that are smaller than it and if it matches anything in this array, 
send it to the next process.
After this process  is done with its' array, it will still receive and throw away any incoming numbers, to avoid locking the previous number, until it sends the sentinel.

### Last
The last process will receive, compare to its' array and save all matching.

### Common
After that all processes have the same code as broadcast has to be called from all.

The number of results is broadcast first, and then one by one all the results are broadcast from the last one. 

After all that each process prints the results for confirmation.