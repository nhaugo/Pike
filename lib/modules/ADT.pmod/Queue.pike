/* $Id: Queue.pike,v 1.4 2001/11/22 20:00:58 nilsson Exp $
 *
 * A simple FIFO queue. 
 */

#pike __REAL_VERSION__
#define QUEUE_SIZE 100

array l;
int head;
int tail;

//! @fixme
//!   Document this function
void create(mixed ...args)
{
  l = args + allocate(QUEUE_SIZE);
  head = sizeof(args);
  tail = 0;
}

void write(mixed item)
{
  put(item);
}

//! @decl void write(mixed item)
//! @decl void put(mixed item)
//! Adds the @[item] to the queue.
//
void put(mixed item)
{
  if (head == sizeof(l))
  {
    l = l[tail ..];
    head -= tail;
    tail = 0;
    l += allocate(sizeof(l) + QUEUE_SIZE);
  }
  l[head++] = item;
//  werror(sprintf("Queue->put: %O\n", l[tail..head-1]));
}

mixed read()
{
  return get();
}

//! @decl mixed read()
//! @decl mixed get()
//! Returns the next element from the queue.
//
mixed get()
{
//  werror(sprintf("Queue->get: %O\n", l[tail..head-1]));
  mixed res;
  if (tail == head)
    return ([])[0];
  res = l[tail];
  l[tail++] = 0;
  return res;
}

//! Returns the next element from the queue
//! without removing it from the queue.
mixed peek()
{
  return (tail < head) && l[tail];
}

//! Returns true if the queue is empty,
//! otherwise zero.
int(0..1) is_empty()
{
  return (tail == head);
}

//! Empties the queue.
void flush()
{
  create();
}
