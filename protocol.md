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
**Auth:** authenticate a db_user  
*Syntax:* ``AUTH u:<username> p:<password>``  
*Responses:*
 - ``OK t:<token>``; authentication succeeded. You're now logged in. Use the ``<token>`` in further requests.
 - ``ERR r:'No such db_user'``; authentication failed. This db_user does not exist.
 - ``ERR r:'Invalid pass'``; authentication failed. Wrong password.
 - ``ERR r:'Banned'``; authentication failed. Your account has been banned.
---

**Exit:** log out from a db_user's account  
*Syntax:* ``AUTH t:<token>``  
*Responses:*
 - ``OK``; logout succeeded. You're now logged out (no valid tokens remain).
 - ``ERR r:'Invalid token'``; logout failed. The given token is invalid.

---

**Load:** read the last 50 messages (or less) from the requested db_channel  
*Syntax:* ``LOAD t:<token> c:<db_channel>``  
*Responses:*
 - ``OK c:<amount of messages> l:<message list>``; load succeeded. ``<message list>`` contains ``<amount of messages>`` messages, which is at most 50. The message list is of the form ``message 1 id;message 2 id;message 3 id``.  
 - ``ERR r:'Invalid token'``; load failed. The given token is invalid.
 - ``ERR r:'Unknown db_channel'``; load failed. The db_channel does not exist, or you don't have rights to see it.

---

**Load-o:** read the last 50 messages, skipping the most recent `n` from the requested db_channel  
*Syntax:* ``LOADO t:<token> c:<db_channel> s:<n>``  
*Responses:*
- ``OK c:<amount of messages> l:<message list>``; load succeeded. ``<message list>`` contains ``<amount of messages>`` messages, which is at most 50. The message list is of the form ``message 1 id;message 2 id;message 3 id``.  
- ``ERR r:'Invalid token'``; load-o failed. The given token is invalid.
- ``ERR r:'Unknown db_channel'``; load-o failed. The db_channel does not exist, or you don't have rights to see it.

---

**Load-m:** loads a given message by id  
*Syntax:* ``LOADM t:<token> m:<message id>``  
*Responses:*
 - ``OK l:<message length> s:<UNIX timestamp representing sent date> c:<message content>``; load-m succeeded. ``<message content>`` is a ``<message length>``-long string.
 - ``ERR r:'Invalid token'``; load-m failed. The given token is invalid.
 - ``ERR r:'Unknown message'``; load-m failed. The message does not exist, or you don't have rights to see the db_channel it was sent in.

---

**Load-c:** loads the db_channel list  
*Syntax:* ``LOADC t:<token>``  
*Responses:*
 - ``OK c:<amount of channels> l:<db_channel list>``; load succeeded. ``<db_channel list>`` contains ``<amount of channels>``. The db_channel list is of the form ``db_channel 1 id;db_channel 2 id;db_channel 3 id``.
 - ``ERR r:'Invalid token'``; load-c failed. The given token is invalid.

---

**Lea:** loads details about a db_channel  
*Syntax:* ``LEA t:<token> c:<db_channel id>``  
*Responses:*
 - ``OK l:<db_channel name length> n:<db_channel name> q:<owner name length> o:<owner name> e:<description length> d:<description>``; load succeeded. All text-based fields have their lengths sent as well.
 - ``ERR r:'Invalid token'``; load failed. The given token is invalid.
 - ``ERR r:'Unknown db_channel'``; load failed. The db_channel does not exist, or you don't have rights to see it.

---

**Store:** sends a message to a db_channel  
*Syntax:* ``STORE t:<token> c:<db_channel> m:<message>``  
*Responses:*
 - ``OK``; store succeeded.
 - ``ERR r:'Invalid token'``; store failed. The given token is invalid.
 - ``ERR r:'Unknown db_channel'``; store failed. The db_channel does not exist, or you don't have rights to send messages to it.

### Global errors
When a message can't be parsed, one of the following is returned:
 - ``ERR r:'Unparsable request'`` when the command or arguments can't be parsed;
 - ``ERR r:'Invalid command'`` when the command can't be used (sending ``OK`` or ``ERR`` as client);
 - ``ERR r:'Argument error' s:'Data type' k:<argument key>`` when an argument is of the wrong type (integer when a string is expected or vice versa);
 - ``ERR r:'Argument error' s:'Missing' k:<(first) missing argument key>`` when one or more arguments are missing.

## Data types
Most values are strings; and these will always be enclosed in single quotes (``'``). IDs and a few other values are integers, which will be sent in decimal, without enclosing quotes.  
Lists are also passed as strings (colon-separated strings). The only types of lists are int-lists (IDs).

In requests, when specifying a db_user or db_channel, the db_user should always send the ID as an integer.
