import java.util.concurrent.*;
import java.util.concurrent.locks.*;

class Config {
    public static final int NUM_PHILOSOPHERS = 5;
}


class Printer {
    private int[] state;
    private int num_rows;

    public Printer(int num_rows){
        this.num_rows = num_rows;
        state = new int[num_rows];
        for(int i = 0; i < num_rows; i++){
            state[i] = -1;
        }
    }

    synchronized public void upd(int calling, int feeding){
        state[calling] = feeding;
        //clear the screen
        {
        System.out.print("\u001B[1J");
        System.out.print("\u001B[H");
        System.out.flush();
        }

        //print the data
        {
        for(int i = 0; i < num_rows; i++){
            if(state[i] != -1)
                System.out.println(i + " is feeding " + state[i]);
            else
                System.out.println(i + " is NOT feeding");
        }

        }
    }
}


class HoldsLockException extends RuntimeException {
    final int locked_num;
    public HoldsLockException(int n){
        super("you tried to feed two people at the same time");
        locked_num = n;
    }
}

class HTable {
    private int num_seats;
    private int feed_next;
    private ReentrantLock[] eating;

    public HTable(int num_seats){
        feed_next = 0;
        this.num_seats = num_seats;
        
        eating = new ReentrantLock[num_seats];
        for(int i = 0; i < num_seats; i++){
            eating[i] = new ReentrantLock();
        }
    }


 //return which am I feed_next with the lock to it
    synchronized public int feed(int my_num) throws HoldsLockException {
        //if a function eneters in here while holding a lock, it means that it didn't finish feeding another guy, illegal
        for(int i = 0; i < num_seats; i++){ 
            if(eating[i].isHeldByCurrentThread()){
                throw new HoldsLockException(i);
            }

            //another way with assert
            //assert eating[i].isHeldByCurrentThread();
        }

        int copy = feed_next;

        while(true){
            if(copy == my_num) //can't feed yourself
                copy = (copy+1)%num_seats;
            
            if(eating[copy].tryLock())  //if got the lock, end loop
                break;

            copy = (copy+1)%num_seats; //otherwise skip to another
        }
        //don't skip yourself, you fed the next available person, but if you were next, let others do that
        if(my_num != feed_next) 
            feed_next = (copy+1)%num_seats; //next one in line

        return copy;
    }


    public void end_feed(int feeded_num){
        eating[feeded_num].unlock();
        return;
    }

}

class Philosopher extends Thread{

    int rounds;
    int easy_num;
    HTable table;
    int feed_next = -1;
    Printer printer;

    public Philosopher(int easy_num, int rounds, HTable table, Printer printer){
        this.easy_num = easy_num;
        this.rounds = rounds;
        this.table = table;
        this.printer = printer;
    }

    public void run() {
        try{
        for(int i = 0; i < rounds; i++){

            try{
            feed_next = table.feed(easy_num);
            printer.upd(easy_num, feed_next);
            //System.out.println("I am " + easy_num + " and I'm feeding " + feed_next);
            Thread.sleep(50*ThreadLocalRandom.current().nextInt(100));   //feed the person for a random time
            printer.upd(easy_num, -1); //call this first so no confusion
            //System.out.println("I am " + easy_num + " and I'm NOT feeding ");
            table.end_feed(feed_next);
            Thread.sleep(50*ThreadLocalRandom.current().nextInt(100));    //philosophize for a sec
            }catch (HoldsLockException e){
                table.end_feed(e.locked_num);
                i--;
            }
            
        }
        } catch (InterruptedException e){
            return;
        }
    }

}



class Main {  
    static HTable table = new HTable(Config.NUM_PHILOSOPHERS);
    static Printer printer = new Printer(Config.NUM_PHILOSOPHERS);
    public static void main(String[] arg) {
        int rounds = Integer.parseInt(arg[0],10);

        for(int i = 0; i < Config.NUM_PHILOSOPHERS; i++){
            new Philosopher(i, rounds, table, printer).start();
        }
    }
}