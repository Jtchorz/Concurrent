import java.util.concurrent.*;
import java.util.concurrent.locks.*;

class Config {
    public static final int NUM_PHILOSOPHERS = 5;
}



class HTable {
    private final Lock lock = new ReentrantLock(true); //this ensures fairness
    private final Lock wait = new ReentrantLock();
    private final Condition partner = wait.newCondition();

    private boolean sleeping = false;
    private boolean is_waitng = false;
    private int sleeping_num;
    private int waking_num;

    private Lock[] eating;

    public HTable(int num_seats){
        eating = new ReentrantLock[num_seats];
        for(int i = 0; i < num_seats; i++){
            eating[i] = new ReentrantLock();
        }
    }


//this method returns with the easy number of the thread we are feeding
//and when you return from it you hold the lock to the one you are feeding

    public int feed(int easy_num) throws InterruptedException{ //return which am I feeding and for how long
        lock.lock(); 
        if(!sleeping){
            sleeping = true;
            sleeping_num = easy_num;  //tell the thread that will come which is waiting to be fed
    
            wait.lock();  //set the inner lock, this is to not let threads outside the monitor compete with this one
            lock.unlock();  //let another thread come to feed each other

            while(!is_waitng) partner.await(); //wait until the other thread signals that we are done

//inside here the lock is still locked, by the other thread
            int to_feed = waking_num; //get the number we are to feed
            sleeping = false;  //condition for the other to wake up safely
            eating[waking_num].lock();
                              
            partner.signal(); //signal we are done modifying
            wait.unlock();  //unlock the lock so that the other one can compete
//to here, when we are returning, we don't know tbh

            return to_feed;  
            
        }
        else{
            wait.lock();  //lock the wait to signal the other one

            is_waitng = true;  //set the parameters
            int to_feed = sleeping_num;
            waking_num = easy_num; // could be Thread.currentThread().getId() but that is kind defeating the point of having a chill number
            eating[sleeping_num].lock();

            partner.signal();  //signal the other partner to start trying to get wait
//actually dont unlock above the fckn await does that

            while(sleeping) partner.await(); //wait until they done, this is to make sure that lock is still locked from outside,
            wait.unlock();  //release this, because the await makes u own it again

            is_waitng = false; 
            lock.unlock(); //finally let any other thread enter
            return to_feed;
        }
        
    }


    public void end_feeding(int feeded_num){
        eating[feeded_num].unlock();
        return;
    }

    public void check_feedable(int my_num){
        eating[my_num].lock();
        eating[my_num].unlock();
        return;
    }

}

class Philosopher extends Thread{

    int rounds;
    int easy_num;
    HTable table;
    int feeding = -1;

    public Philosopher(int easy_num, int rounds, HTable table){
        this.easy_num = easy_num;
        this.rounds = rounds;
        this.table = table;
    }

    public void run() {
        try{
        for(int i = 0; i < rounds; i++){
            feeding = table.feed(easy_num);
            System.out.println("I am " + easy_num + " and I'm feeding " + feeding);
            Thread.sleep(ThreadLocalRandom.current().nextInt(100));   //feed the person for a random time
            table.end_feeding(feeding);
            Thread.sleep(ThreadLocalRandom.current().nextInt(100));    //philosophize for a sec
            table.check_feedable(easy_num); //this is not strictly necessarry but will prevent threads stalling inside the pairing
        }
        }catch (InterruptedException e){
            return;
        }
    }

}



class Main {  
    static HTable table = new HTable(Config.NUM_PHILOSOPHERS);
    public static void main(String[] arg) {
        int rounds = Integer.parseInt(arg[0],10);

        for(int i = 0; i < Config.NUM_PHILOSOPHERS; i++){
            new Philosopher(i, rounds, table).start();
        }
    }
}