document.addEventListener('DOMContentLoaded', function() {
    const baseUrl = `https://localhost:8443`;
    let authToken = null;
    let userData = null;
    
    // Элементы интерфейса
    const pingBtn = document.getElementById('pingBtn');
    const pingStatus = document.getElementById('pingStatus');
    const loginForm = document.getElementById('loginForm');
    const registerForm = document.getElementById('registerForm');
    const loginStatus = document.getElementById('loginStatus');
    const registerStatus = document.getElementById('registerStatus');
    const userSection = document.getElementById('userSection');
    const userToken = document.getElementById('userToken');
    const userRole = document.getElementById('userRole');
    const userName = document.getElementById('userName');
    const logoutBtn = document.getElementById('logoutBtn');
    const tabs = document.querySelectorAll('.tab');
    const tabContents = document.querySelectorAll('.tab-content');
    const registerRole = document.getElementById('registerRole');
    const nameLabel = document.getElementById('nameLabel');
    const nameFieldGroup = document.getElementById('nameFieldGroup');
    
    // Изменение текста метки в зависимости от роли
    registerRole.addEventListener('change', function() {
        if (this.value === 'buyer') {
            nameLabel.textContent = 'Имя и фамилия:';
            document.getElementById('registerName').placeholder = 'Иван Иванов';
        } else if (this.value === 'seller') {
            nameLabel.textContent = 'Название компании:';
            document.getElementById('registerName').placeholder = 'ООО "Рога и копыта"';
        }
    });
    
    // Переключение между вкладками
    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            const tabId = tab.getAttribute('data-tab');
            
            // Деактивируем все вкладки
            tabs.forEach(t => t.classList.remove('active'));
            tabContents.forEach(tc => tc.classList.remove('active'));
            
            // Активируем выбранную вкладку
            tab.classList.add('active');
            document.getElementById(`${tabId}Tab`).classList.add('active');
        });
    });
    
    // Функция для отображения статуса
    function showStatus(element, message, isSuccess) {
        element.textContent = message;
        element.classList.remove('hidden', 'success', 'error');
        element.classList.add(isSuccess ? 'success' : 'error');
    }
    
    // Функция для выполнения запросов к серверу
    async function makeRequest(url, options = {}) {
        try {
            const response = await fetch(url, {
                ...options,
                headers: {
                    'Content-Type': 'application/json',
                    ...options.headers
                }
            });
            
            // Пытаемся получить JSON, если не получается - текст
            let data = await response.text();
            
            return { status: response.status, data };
        } catch (error) {
            return { status: 0, data: `Ошибка: ${error.message}` };
        }
    }
    
    // Проверка соединения (ping)
    pingBtn.addEventListener('click', async () => {
        const { status, data } = await makeRequest(`${baseUrl}/ping`);
        
        if (status === 200) {
            showStatus(pingStatus, `Сервер отвечает: ${data}`, true);
        } else {
            showStatus(pingStatus, `Ошибка: ${typeof data === 'object' ? data.message : data}`, false);
        }
    });
    
    // Форма входа
    loginForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        
        const username = document.getElementById('loginUsername').value;
        const password = document.getElementById('loginPassword').value;
        
        const { status, data } = await makeRequest(`${baseUrl}/api/login`, {
            method: 'POST',
            body: JSON.stringify({ username, password })
        });
        
        if (status === 200) {
            showStatus(loginStatus, 'Успешный вход!', true);
            authToken = data.token;
            userData = data.user;
            
            // Обновляем информацию о пользователе
            userToken.textContent = authToken || 'Токен не получен';
            userRole.textContent = userData?.role || 'Не указана';
            userName.textContent = userData?.name || 'Не указано';
            userSection.classList.remove('hidden');
            
            // Очищаем форму
            loginForm.reset();
        } else {
            showStatus(loginStatus, `Ошибка входа: ${typeof data === 'object' ? data.message : data}`, false);
        }
    });
    
    // Форма регистрации
    registerForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        
        const username = document.getElementById('registerUsername').value;
        const email = document.getElementById('registerEmail').value;
        const password = document.getElementById('registerPassword').value;
        const role = document.getElementById('registerRole').value;
        const name = document.getElementById('registerName').value;
        
        // Валидация роли
        if (!role) {
            showStatus(registerStatus, 'Пожалуйста, выберите роль', false);
            return;
        }
        
        const { status, data } = await makeRequest(`${baseUrl}/api/register`, {
            method: 'POST',
            body: JSON.stringify({ username, email, password, role, name })
        });
        
        if (status === 200 || status === 201) {
            showStatus(registerStatus, 'Регистрация успешна! Теперь вы можете войти.', true);
            registerForm.reset();
            
            // Переключаем на вкладку входа
            tabs.forEach(t => t.classList.remove('active'));
            tabContents.forEach(tc => tc.classList.remove('active'));
            document.querySelector('[data-tab="login"]').classList.add('active');
            document.getElementById('loginTab').classList.add('active');
        } else {
            showStatus(registerStatus, `Ошибка регистрации: ${typeof data === 'object' ? data.message : data}`, false);
        }
    });
    
    // Выход
    logoutBtn.addEventListener('click', () => {
        authToken = null;
        userData = null;
        userSection.classList.add('hidden');
        userToken.textContent = '';
        userRole.textContent = '';
        userName.textContent = '';
    });
    
    // Предупреждение о самоподписанном сертификате
    console.warn('Внимание: используется самоподписанный сертификат. Браузер может блокировать запросы.');
    alert('Внимание: используется самоподписанный сертификат. Для работы с сайтом вам нужно разрешить доступ к небезопасному содержимому.');
});