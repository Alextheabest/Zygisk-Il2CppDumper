const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// Serve static files from public directory
app.use(express.static(path.join(__dirname, 'public')));

// Store connected users and messages
const users = new Map();
const messages = [];

// Socket.io connection handling
io.on('connection', (socket) => {
  console.log('A user connected:', socket.id);

  // Handle user joining
  socket.on('join', (username) => {
    if (!username || username.trim() === '') {
      socket.emit('error', 'Username is required');
      return;
    }
    
    users.set(socket.id, {
      id: socket.id,
      username: username.trim(),
      joinTime: new Date()
    });
    
    // Send existing messages to the new user
    socket.emit('message history', messages);
    
    // Broadcast user joined
    socket.broadcast.emit('user joined', {
      username: username.trim(),
      timestamp: new Date()
    });
    
    // Send updated user list
    io.emit('users update', Array.from(users.values()));
    
    console.log(`User ${username} joined`);
  });

  // Handle incoming messages
  socket.on('chat message', (data) => {
    const user = users.get(socket.id);
    if (!user) {
      socket.emit('error', 'Please join the chat first');
      return;
    }

    const message = {
      id: Date.now(),
      username: user.username,
      message: data.message,
      timestamp: new Date()
    };
    
    messages.push(message);
    
    // Keep only last 100 messages
    if (messages.length > 100) {
      messages.shift();
    }
    
    // Broadcast message to all users
    io.emit('chat message', message);
    
    console.log(`Message from ${user.username}: ${data.message}`);
  });

  // Handle user typing
  socket.on('typing', (data) => {
    const user = users.get(socket.id);
    if (user) {
      socket.broadcast.emit('typing', {
        username: user.username,
        isTyping: data.isTyping
      });
    }
  });

  // Handle disconnection
  socket.on('disconnect', () => {
    const user = users.get(socket.id);
    if (user) {
      users.delete(socket.id);
      
      // Broadcast user left
      socket.broadcast.emit('user left', {
        username: user.username,
        timestamp: new Date()
      });
      
      // Send updated user list
      io.emit('users update', Array.from(users.values()));
      
      console.log(`User ${user.username} disconnected`);
    }
  });
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`Chat server running on port ${PORT}`);
  console.log(`Open http://localhost:${PORT} to access the chat`);
});