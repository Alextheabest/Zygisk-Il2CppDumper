# Web Chat App

A modern, real-time web chat application built with Node.js, Express, Socket.io, and vanilla JavaScript.

## Features

- **Real-time messaging** - Messages appear instantly for all connected users
- **User authentication** - Simple username-based login system
- **Online user list** - See who's currently online
- **Typing indicators** - Know when someone is typing
- **Message history** - New users see previous messages (last 100)
- **Responsive design** - Works on desktop and mobile devices
- **Modern UI** - Clean, gradient-based design with smooth animations
- **Connection status** - Visual indicator of connection state

## Tech Stack

- **Backend**: Node.js, Express.js, Socket.io
- **Frontend**: HTML5, CSS3, JavaScript (ES6+)
- **Real-time Communication**: WebSockets via Socket.io
- **Styling**: Custom CSS with gradient themes and Font Awesome icons

## Installation

1. Install Node.js (if not already installed)
2. Install dependencies:
   ```bash
   npm install
   ```

## Running the Application

1. Start the server:
   ```bash
   npm start
   ```

2. Open your browser and navigate to:
   ```
   http://localhost:3000
   ```

3. Enter a username and start chatting!

## Development Mode

For development with auto-restart:
```bash
npm run dev
```

## How to Use

1. **Join Chat**: Enter a username (max 20 characters) and click "Join Chat"
2. **Send Messages**: Type your message and press Enter or click the send button
3. **View Users**: Click the "Users" button to see who's online
4. **Real-time Updates**: Messages appear instantly, typing indicators show when others are typing

## Features in Detail

### Real-time Messaging
- Messages are delivered instantly to all connected users
- Message history is preserved for new users joining the chat
- Only the last 100 messages are kept in memory

### User Management
- Username validation (required, max 20 characters)
- Online user tracking
- Join/leave notifications

### UI/UX Features
- Responsive design that works on all devices
- Modern gradient theme with smooth animations
- Typing indicators to show when users are typing
- Connection status indicator
- Auto-scroll to latest messages
- Error notifications for better user feedback

### Technical Features
- WebSocket-based real-time communication
- Automatic reconnection handling
- XSS protection with HTML escaping
- Mobile-optimized touch interactions

## Browser Support

- Chrome (recommended)
- Firefox
- Safari
- Edge

## License

MIT License

## Contributing

Feel free to submit issues and pull requests to improve the application.