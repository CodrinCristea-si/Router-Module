const server_location="http://127.0.0.1:5000/"
function fetch_clients(){
     fetch(server_location +'clients')
         .then(response => response.text())
         .then(data => {
             console.log(data)
            document.getElementById('content').innerHTML = data;
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
function toggleList() {
    var sidebar = document.getElementById("sidebar");
    sidebar.classList.toggle("show");
}
window.addEventListener('load',()=>{
    toggleList();
})