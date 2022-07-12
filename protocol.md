# Protocol description

## Message structure
### Client messages
Every message should have the following structure:
```
<len><LF>
<command> <arguments>
```

| **Field**     | **Width in bytes** | **Meaning**                                                |
|---------------|--------------------|------------------------------------------------------------|
| `<len>`       | 4 bytes            | The length of the command and its arguments as a uint32\_t |
| `<LF>`        | 1 byte             | A single (literal) newline (line-feed character)           |
| `<command>`   | ??? bytes          | One of the commands below, in capitals                     |
| `<arguments>` | ??? bytes          | The arguments of the command, in the form `key:value`      |

The total byte-size of ``<command>`` and ``<arguments>`` (plus the joining space) is equal to ``<len>``, making the full message of length 4 + 1 + ``<len>``.  

### Server messages
Server messages follow the same structure as client messages, except for their commands. These are either
 - ``OK`` to indicate success (with the arguments representing the data passed, if any); or
 - ``ERR`` to indicate failure (with the ``r`` argument giving a diagnostic, on which further arguments might shed more light).

## Commands
**Auth ():** authenticate a user  
*Syntax:* ``AUTH u:<username> p:<password>``  
*Responses:*
 - ``OK t:<token>``; authentication succeeded. You're now logged in. Use the ``<token>`` in further requests.
 - ``ERR r:'No such user'``; authentication failed. This user does not exist.
 - ``ERR r:'Invalid pass'``; authentication failed. Wrong password.
 - ``ERR r:'Banned'``; authentication failed. Your account has been banned.
---

**Exit:** log out from a user's account  
*Syntax:* ``AUTH t:<token>``  
*Responses:*
 - ``OK``; logout succeeded. You're now logged out (no valid tokens remain).
 - ``ERR r:'Invalid token'``; logout failed. The given token is invalid.

---

**Load:** read the last 50 messages (or less) from the requested channel  
*Syntax:* ``LOAD t:<token> c:<channel>``  
*Responses:*
 - ``OK c:<amount of messages> l:<message list>``; load succeeded. ``<message list>`` contains ``<amount of messages>`` messages, which is at most 50. The message list is of the form ``message 1 id;message 2 id;message 3 id``.  
 - ``ERR r:'Invalid token'``; load failed. The given token is invalid.
 - ``ERR r:'Unknown channel'``; load failed. The channel does not exist, or you don't have rights to see it.

---

**Load-o:** read the last 50 messages, skipping the most recent `n` from the requested channel  
*Syntax:* ``LOAD-O t:<token> c:<channel> s:<n>``  
*Responses:*
- ``OK c:<amount of messages> l:<message list>``; load succeeded. ``<message list>`` contains ``<amount of messages>`` messages, which is at most 50. The message list is of the form ``message 1 id;message 2 id;message 3 id``.  
- ``ERR r:'Invalid token'``; load-o failed. The given token is invalid.
- ``ERR r:'Unknown channel'``; load-o failed. The channel does not exist, or you don't have rights to see it.

---

**Load-m:** loads a given message by id  
*Syntax:* ``LOAD-M t:<token> m:<message id>``  
*Responses:*
 - ``OK l:<message length> s:<UNIX timestamp representing sent date> c:<message content>``; load-m succeeded. ``<message content>`` is a ``<message length>``-long string.
 - ``ERR r:'Invalid token'``; load-m failed. The given token is invalid.
 - ``ERR r:'Unknown message'``; load-m failed. The message does not exist, or you don't have rights to see the channel it was sent in.

---

**Store:** sends a message to a channel  
*Syntax:* ``STORE t:<token> c:<channel> m:<message>``  
*Responses:*
 - ``OK``; store succeeded.
 - ``ERR r:'Invalid token'``; store failed. The given token is invalid.
 - ``ERR r:'Unknown channel'``; store failed. The channel does not exist, or you don't have rights to send messages to it.

### Global errors
When a message can't be parsed, one of the following is returned:
 - ``ERR r:'Unknown command'`` when the command can't be parsed;
 - ``ERR r:'Argument error' s:'Unknown key' a:<argument number>`` when an argument key (the letter before the ``:``) can't be parsed or is invalid;
 - ``ERR r:'Argument error' s:'Data type' a:<argument number> k:<argument key>`` when an argument is of the wrong type (integer when a string is expected or vice versa);
 - ``ERR r:'Argument error' s:'Missing' k:<(first) missing argument key>`` when one or more arguments are missing.

## Data types
Most values are strings; and these will always be enclosed in single quotes (``'``). IDs and a few other values are integers, which will be sent in decimal, without enclosing quotes.

In requests, when specifying a user or channel, you can choose between:
 - Sending the name of the user/channel (as a string in single quotes); or
 - Sending the ID of the user/channel (as an integer).
