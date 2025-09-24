let messages=[];
let peoples=[];
function send(){
    const name = document.getElementById('name').value;
    const password = document.getElementById('password').value;
    const message = document.getElementById('message').value;
    const toUsername = document.getElementById('toUsername').value;

    fetch(window.location.pathname, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ "username" : name, "password" : password, "message" : message,"toUsername" : toUsername })
    })
    .then(response => response.json())
    .then(data => console.log(data))
    .catch(error => console.error('Error:', error));
}

function get(){
    const name = document.getElementById('name').value;
    const password = document.getElementById('password').value;

    fetch('/read', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({"username" :name, "password" :password })
    })
    .then(response => response.text())
    .then(data => {
        messages= JSON.parse(data);
    })
    .catch(error => console.error('Error:', error));
    showmessages();
}
function showmessages() {
    messages.forEach(message => {
        if (message["user"] && message["message"] && peoples.indexOf(message["user"]) === -1){
            peoples.push(message["user"]);
        }
    });
    let lst = document.querySelector('.Peoples');
    lst.innerHTML = "";
    peoples.forEach(person => {
        let h3 = document.createElement("button");
        h3.innerText = person;
        h3.className = "person";
        h3.onclick = "showMessage(" + person + ")";
        lst.appendChild(h3);
    })
}
function showMessage(Person) {
    let lst = document.querySelector('.messages');
    lst.innerHTML = "";
    messages.forEach(message => {
        if (message["user"]==Person && message["message"]){
            if (message["me"]){
                lst.innerHTML += "<p class = \"messageMe\">" + message["message"] + "<br></p>";
            }
            else {
                lst.innerHTML += "<p class = \"messageOther\">" + message["message"] + "<br></p>";
            }
        }
    });
}
function openPerson(person) {
    const messageDiv = document.querySelector('.messages');
    messageDiv.innerHTML = "";
    messages.forEach(message => {
        if (message["user"] === person){
            messageDiv.innerHTML += "<p class = \"messageOther\">" + message["message"] + "<br></p>";
        }
    });
}