
function fill_hist_content(data){
     let hist_content = document.getElementById("content_history")
    hist_content.innerHTML = ""
     for (let el of data){
         hist_content.innerHTML += el+"<br>"
     }
}

function load_sys_history(nr_hist){
    const server_location="http://127.0.0.1:5000/"
     fetch(server_location +"history?type=2&nr="+nr_hist)
         .then(response => response.text())
         .then(data => {
             console.log(data)
             data = JSON.parse(data)
             fill_hist_content(data)
        }).catch(err => {
            document.getElementById("content_history").textContent = err;
     })
}

function load_sys_logs(nr_hist){
    const server_location="http://127.0.0.1:5000/"
     fetch(server_location +"history?type=1&nr="+nr_hist)
         .then(response => response.text())
         .then(data => {
             console.log(data)
             data = JSON.parse(data)
             fill_hist_content(data)
        }).catch(err => {
            document.getElementById("content_history").textContent = err;
     })
}

function begin(){
    let nr_ent = 100
    let but_hist = document.getElementById("button_history");
    but_hist.addEventListener("click",function (){
        console.log("click hist");
        load_sys_history(nr_ent);
    });
    let but_logs = document.getElementById("button_logs");
    but_logs.addEventListener("click",function (){
        console.log("click log");
        load_sys_logs(nr_ent);
    });
    load_sys_history(nr_ent);
}

begin();