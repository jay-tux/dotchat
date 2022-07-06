R"SQL_SCRIPT(
CREATE TABLE IF NOT EXISTS users(
    id INT PRIMARY KEY NOT NULL,
    username TEXT UNIQUE NOT NULL,
    pass TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS users_keys(
    session_key TEXT PRIMARY KEY NOT NULL,
    user INT NOT NULL,
    valid_until INT NOT NULL
);

CREATE TABLE IF NOT EXISTS channels(
    id INT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    owner INT NOT NULL,
    description TEXT,
    FOREIGN KEY(owner) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS channel_users(
    user_id INT NOT NULL,
    channel_id INT NOT NULL,
    FOREIGN KEY(user_id) REFERENCES users(id),
    FOREIGN KEY(channel_id) REFERENCES channels(id),
    PRIMARY KEY(user_id, channel_id)
);

CREATE TABLE IF NOT EXISTS messages(
    id INT NOT NULL PRIMARY KEY,
    sender INT NOT NULL,
    channel INT NOT NULL,
    content TEXT NOT NULL,
    date INT NOT NULL,
    FOREIGN KEY(sender) REFERENCES users(id),
    FOREIGN KEY(channel) REFERENCES channels(id)
);

INSERT OR IGNORE INTO users(id, username, pass)
    VALUES(1, 'master', '');
INSERT OR IGNORE INTO channels(id, name, owner, description)
    VALUES(1, 'general', 1, 'The main lobby');
INSERT OR IGNORE INTO channel_users(user_id, channel_id)
    VALUES(1, 1);
)SQL_SCRIPT";