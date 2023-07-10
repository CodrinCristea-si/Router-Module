const server_location="http://127.0.0.1:5000/"

function fetch_home(){
     fetch(server_location +'home?type=0&state=0')
         .then(response => response.text())
         .then(data => {
             console.log(data)
            let div_content= document.getElementById('content');
             div_content.innerHTML = data;
             var scripts = div_content.getElementsByTagName('script');
            for (var i = 0; i < scripts.length; i++) {
                eval(scripts[i].innerText);
            }
        }).catch(err => {
            document.getElementById('content').textContent = err;
     })
}

function fetch_clients(){
     fetch(server_location +'clients?type=0')
         .then(response => response.text())
         .then(data => {
             console.log(data)
            let div_content= document.getElementById('content');
             div_content.innerHTML = data;
             var scripts = div_content.getElementsByTagName('script');
            for (var i = 0; i < scripts.length; i++) {
                eval(scripts[i].innerText);
            }
        }).catch(err => {
            document.getElementById('content').textContent = err;
     })
}

function fetch_malware_data(){
     fetch(server_location +'malware/0')
         .then(response => response.text())
         .then(data => {
             console.log(data)
             let div_content= document.getElementById('content');
             div_content.innerHTML = data;
             var scripts = div_content.getElementsByTagName('script');
            for (var i = 0; i < scripts.length; i++) {
                eval(scripts[i].innerText);
            }
             // let domContentLoadedEvent = new Event('DOMContentLoaded');
             // document.dispatchEvent(domContentLoadedEvent);
        }).catch(err => {
            document.getElementById('content').textContent = err;
     })
}

function fetch_heuristics(){
     fetch(server_location +'heuristics')
         .then(response => response.text())
         .then(data => {
             console.log(data)
             let div_content= document.getElementById('content');
             div_content.innerHTML = data;
             var scripts = div_content.getElementsByTagName('script');
            for (var i = 0; i < scripts.length; i++) {
                eval(scripts[i].innerText);
            }
             // let domContentLoadedEvent = new Event('DOMContentLoaded');
             // document.dispatchEvent(domContentLoadedEvent);
        }).catch(err => {
            document.getElementById('content').textContent = err;
     })
}

function fetch_history(){
     fetch(server_location +"history?type=0&nr=0")
         .then(response => response.text())
         .then(data => {
             console.log(data)
             let div_content= document.getElementById('content');
             div_content.innerHTML = data;
             var scripts = div_content.getElementsByTagName('script');
            for (var i = 0; i < scripts.length; i++) {
                eval(scripts[i].innerText);
            }
             // let domContentLoadedEvent = new Event('DOMContentLoaded');
             // document.dispatchEvent(domContentLoadedEvent);
        }).catch(err => {
            document.getElementById('content').textContent = err;
     })
}
function toggleList() {
    var sidebar = document.getElementById("sidebar");
    sidebar.classList.toggle("show");
}
window.addEventListener('load',()=>{
    toggleList();
    fetch_home();
})