## A collection of real time systems projects executed using Keil µVision 5.


## Notable Project: Byzantine Generals Problem ##

Real-time systems face challenges coordinating distributed components reliably over unreliable networks, crucial for unanimous decisions despite potential faults or deceitful information. The Byzantine Generals' Problem is an analogy used to describe the difficulties of reaching consensus in distributed systems.

In this implementation, mutexes and semaphores are utilized in thread synchronization to solve the Byzantine Generals' Problem by managing critical code sections and coordinating shared resource access despite unreliable communication or potential traitorous messages.

The Byzantine Generals' Problem involves a group of generals coordinating their attack or retreat plans through unreliable communication channels while accounting for potential traitorous generals, aiming to reach a consensus despite conflicting information.
In this scenario, a Commander communicates a message to several generals, instructing them to either attack or retreat. However, some of these generals are traitorous and might alter the received command to deceive the others.

**How the Code Works**

A new thread is created for each general. Each of these generals has a queue which the other generals use to relay their messages to them with. The generals wait for the commanders message which is sent via the `broadcast()` function, then they use the OM (Oral Message) algorithm to solve the Byzantine General's Problem. A general is designated as the reporter which outputs the messages they receive in OM(0).

The OM algorithm is a recursive function enabling communication between general threads. Initially, the general queues hold a message from the Commander. Each general sends the command it received from the commander to all of the other generals. However, traitorous generals alter every received command before sharing it. Then a reusable two-phase barrier is used to synchronize all of the general threads. This blocks the general threads until all of the generals have finished sending their messages. Once this occurs, the barrier allows all of the threads to continue. Then, the generals read their queues and recursively call OM(order-1) to share messages among all generals.

In this implementation, the utilization of mutexes and semaphores showcases a robust solution to the Byzantine Generals' Problem. Through the OM algorithm, each general efficiently relays and processes messages despite potential traitorous interference, ensuring reliable consensus among distributed components in real-time systems.

