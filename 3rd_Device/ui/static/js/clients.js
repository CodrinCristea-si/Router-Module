
function fetch_test(ip,mac){
    const server_location="http://127.0.0.1:5000/"
     fetch(server_location +'test_clients?ip='+ip +"&mac=" +mac +"&type=0")
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

function fetch_packages(ip, mac){
    const server_location="http://127.0.0.1:5000/"
     fetch(server_location +'package_clients?ip='+ip +"&mac=" +mac +"&nr=0"+"&type=0")
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

function reach_client(ip, mac,button){
    const server_location="http://127.0.0.1:5000/"
     fetch(server_location +'clients?ip='+ip +"&type=1")
         .then(response => response.text())
         .then(data => {
             console.log(data)
             data = JSON.parse(data)
             if (data["reach"]){
                 alert("Client with IP "+ ip + " has been reached")
             }
             else{
                 alert("Client with IP "+ ip + " cannot be reached")
             }

        }).catch(err => {
            document.getElementById('content').textContent = err;
     })
    button.disabled = false
}

function refresh_clients(){
    const server_location="http://127.0.0.1:5000/"
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

function scan_client(ip,mac,button){
    const server_location="http://127.0.0.1:5000/"
     fetch(server_location +'clients?ip='+ip +"&type=2")
         .then(response => response.text())
         .then(data => {
             console.log(data)
             data = JSON.parse(data)
             if (data["scan"]){
                 alert("Test scan for "+ ip + " has begun with success")
             }
             else{
                 alert("Test scan for "+ ip + " has begun with failure")
             }
             refresh_clients()
        }).catch(err => {
            document.getElementById('content').textContent = err;
     })
    button.disabled = false
}

function transfer_client(ip, mac, score, infec_type){
    data= {
        "ip":ip,
        "mac":mac,
        "score": score,
        "infec_type":infec_type
    }
    const server_location="http://127.0.0.1:5000/"
     fetch(server_location +'clients?type=3', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json'
            },
            body: JSON.stringify(data)
     })
         .then(response => response.text())
         .then(data => {
             console.log(data)
             data = JSON.parse(data)
             if (data["scan"]){
                 alert("Transfer for "+ ip + " finished with success")
             }
             else{
                 alert("Transfer for "+ ip + " finished with failure")
             }
             refresh_clients();
        }).catch(err => {
            document.getElementById('content').textContent = err;
     })
}

function begin(){

    let select_is_testing=document.querySelectorAll(".selection-testing");
     select_is_testing.forEach(function(item) {
        item.addEventListener("change",function (){
            let item_selected = item.options[item.selectedIndex];
            item.className = item.className.replaceAll("select-yes","")
            item.className = item.className.replaceAll("select-no","")
            item.className += " " + item_selected.className;
        })
      });

     let select_infectivity=document.querySelectorAll(".selection-infectivity");
     select_infectivity.forEach(function(item) {
        item.addEventListener("change",function (){
            let item_selected = item.options[item.selectedIndex];
            item.className = item.className.replaceAll("element infectivity uninfected","")
            item.className = item.className.replaceAll("element infectivity suspect","")
            item.className = item.className.replaceAll("element infectivity minor","")
            item.className = item.className.replaceAll("element infectivity major","")
            item.className = item.className.replaceAll("element infectivity sever","")
            item.className = item.className.replaceAll("select-no","")
            item.className += " " + item_selected.className;
        })
      });
    let list_butt_see_tests=document.querySelectorAll(".see_tests");
    console.log("begin");
    list_butt_see_tests.forEach(function(item) {
        item.addEventListener("click",function (){
            console.log("click see tests");
           tr_item = this.parentNode.parentNode.parentNode.parentNode;
           det = tr_item.getAttribute("id");
           ip_addr = det.split("|")[0]
           mac_addr = det.split("|")[1]
           fetch_test(ip_addr, mac_addr);
        })
      });
    let list_butt_see_packages=document.querySelectorAll(".see_packages");
    list_butt_see_packages.forEach(function(item) {
        item.addEventListener("click",function (){
            console.log("click see packages");
           tr_item = this.parentNode.parentNode.parentNode.parentNode;
           det = tr_item.getAttribute("id");
           ip_addr = det.split("|")[0]
           mac_addr = det.split("|")[1]
           fetch_packages(ip_addr, mac_addr);
        })
      });
    let list_butt_reach=document.querySelectorAll(".reach");
    list_butt_reach.forEach(function(item) {
        item.addEventListener("click",function (){
            console.log("click reach");
           tr_item = this.parentNode.parentNode.parentNode.parentNode;
           det = tr_item.getAttribute("id");
           ip_addr = det.split("|")[0]
           mac_addr = det.split("|")[1]
           reach_client(ip_addr,mac_addr,item);
           item.disabled = true
        })
      });
    let list_butt_scan=document.querySelectorAll(".test_cl");
    list_butt_scan.forEach(function(item) {
        item.addEventListener("click",function (){
            console.log("click scan");
           tr_item = this.parentNode.parentNode.parentNode.parentNode;
           det = tr_item.getAttribute("id");
           ip_addr = det.split("|")[0]
           mac_addr = det.split("|")[1]
           scan_client(ip_addr,mac_addr,item);
            item.disabled = true
        })
      });
    let list_butt_save=document.querySelectorAll(".save");
    list_butt_save.forEach(function(item) {
        item.addEventListener("click",function (){
            console.log("click save");
           tr_item = this.parentNode.parentNode.parentNode.parentNode;
           det = tr_item.getAttribute("id");
           ip_addr = det.split("|")[0]
           mac_addr = det.split("|")[1]
            score = document.getElementById("score|"+det).value;
           if (score <= 0){
               alert("Score should not be negative");
               refresh_clients();
               return;
           }
           let select_infec = document.getElementById("infec_type|"+det);
           let item_selected = select_infec.options[select_infec.selectedIndex];
           infec_type = item_selected.value;
           console.log(ip_addr,mac_addr,score,infec_type)
            transfer_client(ip_addr,mac_addr,score,infec_type)
        })
      });
}

begin();