#include <iostream>
#include <random>
#include <chrono>
#include <stdlib.h>
#include <deque>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex>uLock(_mutex); // perform queue modification under the lock 
    _cond.wait(uLock, [this] {return !_queue.empty(); }); 
    // remove last element from the queue
    T msg = std::move(_queue.back());
    std::cout<<"queue length:"<<_queue.size()<<std::endl;
    _queue.pop_back();
    // _queue.clear();
    return std::move(msg);
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.emplace_back(msg);
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    std::cout<<" Traffic Light #" << getID() << " is created "<<std::endl;
    _currentPhase = TrafficLightPhase::red;
    // _msg = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns. (returns what???)
    while(true)
    {
        std::cout<<" WaitforFreen ->" << _msg.receive() << "  \n";
        TrafficLightPhase phase = _msg.receive();
        if (phase  == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // generate random values
    std::random_device rd; 
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> dist(4000, 6000);
    double cycleDuration = dist(eng);
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    // duration of a single simulation cycle in ms
    // std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    std::chrono::system_clock::time_point lastUpdate;
    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            TrafficLightPhase cyclePhase = (_currentPhase==red)? green:red; // toggle b/t red and green, notice create a seperate trafficLightPhase to store, in order to not modify "this->_currentPhase" 
            _currentPhase = (_currentPhase==red)? green:red; // toggle b/t red and green 
//??? why here give value to _current phase again????????
            _currentPhase = cyclePhase;
            // print confirmation 
            std::cout<<"Traffic Light #"<<getID() <<"turning to "<<(_currentPhase==TrafficLightPhase::red?"red":"green")<<":"<<(cyclePhase==TrafficLightPhase::red?"red":"green")<<std::endl;
            // cycleDuration = rand() % 3000 + 4000;// randomly wait for 4~6 sec
            // MessageQueue<TrafficLightPhase> msg;
            _msg.send(std::move(cyclePhase));// send it to message queue
            cycleDuration = dist(eng);
            lastUpdate = std::chrono::system_clock::now();
        }

        // std::cout<<" Time " << cycleDuration << "  \n";



    }
}

