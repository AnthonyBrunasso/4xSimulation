echo -n "" > game_state

COUNTER=1200
 while [  $COUNTER -lt 1204 ]; do
     echo "Client input received on port " $COUNTER
     nc -vl $COUNTER >> game_state & 
     let COUNTER=COUNTER+1 
 done

COUNTER=4000
 while [  $COUNTER -lt 4004 ]; do
     echo "Client game_state available on port " $COUNTER
     tail -n +1 -f game_state | nc -vl $COUNTER &
     let COUNTER=COUNTER+1 
 done
