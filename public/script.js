// Chat Application Client-Side JavaScript

class ChatApp {
    constructor() {
        this.socket = null;
        this.currentUser = null;
        this.typingUsers = new Set();
        this.typingTimeout = null;
        this.isTyping = false;
        
        this.initializeElements();
        this.initializeSocket();
        this.bindEvents();
    }

    initializeElements() {
        // Modal elements
        this.loginModal = document.getElementById('loginModal');
        this.usernameInput = document.getElementById('usernameInput');
        this.joinBtn = document.getElementById('joinBtn');
        
        // Chat elements
        this.messagesContainer = document.getElementById('messages');
        this.messageInput = document.getElementById('messageInput');
        this.sendBtn = document.getElementById('sendBtn');
        this.onlineCount = document.getElementById('onlineCount');
        this.usersList = document.getElementById('usersList');
        this.usersSidebar = document.getElementById('usersSidebar');
        this.toggleUsersBtn = document.getElementById('toggleUsers');
        this.typingIndicator = document.getElementById('typingIndicator');
        this.connectionStatus = document.getElementById('connectionStatus');
    }

    initializeSocket() {
        this.socket = io();
        
        // Connection events
        this.socket.on('connect', () => {
            this.updateConnectionStatus('connected', 'Connected');
            console.log('Connected to server');
        });

        this.socket.on('disconnect', () => {
            this.updateConnectionStatus('disconnected', 'Disconnected');
            console.log('Disconnected from server');
        });

        this.socket.on('connect_error', () => {
            this.updateConnectionStatus('disconnected', 'Connection Error');
        });

        // Chat events
        this.socket.on('message history', (messages) => {
            this.loadMessageHistory(messages);
        });

        this.socket.on('chat message', (message) => {
            this.displayMessage(message);
        });

        this.socket.on('user joined', (data) => {
            this.displaySystemMessage(`${data.username} joined the chat`);
        });

        this.socket.on('user left', (data) => {
            this.displaySystemMessage(`${data.username} left the chat`);
        });

        this.socket.on('users update', (users) => {
            this.updateUsersList(users);
        });

        this.socket.on('typing', (data) => {
            this.handleTypingIndicator(data);
        });

        this.socket.on('error', (error) => {
            this.showError(error);
        });
    }

    bindEvents() {
        // Login events
        this.joinBtn.addEventListener('click', () => this.joinChat());
        this.usernameInput.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                this.joinChat();
            }
        });

        // Message events
        this.sendBtn.addEventListener('click', () => this.sendMessage());
        this.messageInput.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                this.sendMessage();
            }
        });

        // Typing indicator
        this.messageInput.addEventListener('input', () => {
            this.handleTyping();
        });

        // UI events
        this.toggleUsersBtn.addEventListener('click', () => this.toggleUsersSidebar());
        
        // Close users sidebar when clicking outside on mobile
        document.addEventListener('click', (e) => {
            if (window.innerWidth <= 768 && 
                !this.usersSidebar.contains(e.target) && 
                !this.toggleUsersBtn.contains(e.target)) {
                this.usersSidebar.classList.remove('show');
            }
        });
    }

    joinChat() {
        const username = this.usernameInput.value.trim();
        if (!username) {
            this.showError('Please enter a username');
            return;
        }

        if (username.length > 20) {
            this.showError('Username must be 20 characters or less');
            return;
        }

        this.currentUser = username;
        this.socket.emit('join', username);
        
        this.loginModal.style.display = 'none';
        this.messageInput.disabled = false;
        this.sendBtn.disabled = false;
        this.messageInput.focus();
    }

    sendMessage() {
        const message = this.messageInput.value.trim();
        if (!message) return;

        this.socket.emit('chat message', { message });
        this.messageInput.value = '';
        this.stopTyping();
    }

    displayMessage(message) {
        const messageElement = document.createElement('div');
        messageElement.classList.add('message');
        
        if (message.username === this.currentUser) {
            messageElement.classList.add('own');
        }

        const timestamp = new Date(message.timestamp).toLocaleTimeString([], {
            hour: '2-digit',
            minute: '2-digit'
        });

        messageElement.innerHTML = `
            <div class="message-content">
                <div class="message-header">
                    <span class="message-username">${this.escapeHtml(message.username)}</span>
                    <span class="message-timestamp">${timestamp}</span>
                </div>
                <div class="message-text">${this.escapeHtml(message.message)}</div>
            </div>
        `;

        this.messagesContainer.appendChild(messageElement);
        this.scrollToBottom();
    }

    displaySystemMessage(message) {
        const messageElement = document.createElement('div');
        messageElement.classList.add('system-message');
        messageElement.textContent = message;
        
        this.messagesContainer.appendChild(messageElement);
        this.scrollToBottom();
    }

    loadMessageHistory(messages) {
        this.messagesContainer.innerHTML = '';
        messages.forEach(message => {
            this.displayMessage(message);
        });
    }

    updateUsersList(users) {
        this.onlineCount.textContent = `${users.length} user${users.length !== 1 ? 's' : ''} online`;
        
        this.usersList.innerHTML = '';
        users.forEach(user => {
            const userElement = document.createElement('div');
            userElement.classList.add('user-item');
            userElement.innerHTML = `
                <i class="fas fa-circle"></i>
                <span>${this.escapeHtml(user.username)}</span>
            `;
            this.usersList.appendChild(userElement);
        });
    }

    handleTyping() {
        if (!this.isTyping) {
            this.isTyping = true;
            this.socket.emit('typing', { isTyping: true });
        }

        clearTimeout(this.typingTimeout);
        this.typingTimeout = setTimeout(() => {
            this.stopTyping();
        }, 2000);
    }

    stopTyping() {
        if (this.isTyping) {
            this.isTyping = false;
            this.socket.emit('typing', { isTyping: false });
        }
        clearTimeout(this.typingTimeout);
    }

    handleTypingIndicator(data) {
        if (data.isTyping) {
            this.typingUsers.add(data.username);
        } else {
            this.typingUsers.delete(data.username);
        }
        this.updateTypingIndicator();
    }

    updateTypingIndicator() {
        const typingArray = Array.from(this.typingUsers);
        let text = '';
        
        if (typingArray.length > 0) {
            if (typingArray.length === 1) {
                text = `${typingArray[0]} is typing...`;
            } else if (typingArray.length === 2) {
                text = `${typingArray[0]} and ${typingArray[1]} are typing...`;
            } else {
                text = `${typingArray.slice(0, -1).join(', ')} and ${typingArray[typingArray.length - 1]} are typing...`;
            }
        }
        
        this.typingIndicator.textContent = text;
    }

    toggleUsersSidebar() {
        this.usersSidebar.classList.toggle('show');
    }

    updateConnectionStatus(status, message) {
        this.connectionStatus.className = `connection-status ${status}`;
        this.connectionStatus.querySelector('span').textContent = message;
    }

    showError(message) {
        // Create a simple error notification
        const errorDiv = document.createElement('div');
        errorDiv.style.cssText = `
            position: fixed;
            top: 20px;
            right: 20px;
            background: #dc3545;
            color: white;
            padding: 1rem;
            border-radius: 10px;
            box-shadow: 0 5px 15px rgba(220, 53, 69, 0.3);
            z-index: 1001;
            animation: fadeIn 0.3s ease;
        `;
        errorDiv.textContent = message;
        
        document.body.appendChild(errorDiv);
        
        setTimeout(() => {
            errorDiv.remove();
        }, 3000);
    }

    scrollToBottom() {
        this.messagesContainer.scrollTop = this.messagesContainer.scrollHeight;
    }

    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}

// Initialize the chat app when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new ChatApp();
});

// Add some nice touch interactions for mobile
if ('ontouchstart' in window) {
    document.addEventListener('touchstart', function() {}, { passive: true });
}