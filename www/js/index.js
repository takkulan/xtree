// Application initialization: we need both jQuery and cordova ready
// first jQuery doc ready then cordova
$(document).ready(function () {
    // do document ready stuff
    console.log('---- Doc ready-----');
    // !!! uncomment two following lines for Cordova. Now this works with standlone chrome !!!
}).on('deviceready', function () {
    console.log('---- Device ready -----');
    initLogin.init(); // carry out autologin if possible or show UI
    xTree.init();
    //    setTimeout(function () {
    //        if (initLogin.testLogin()) deviceChooser.init();
    //    }, 500); // lets give login some leverage (bad solution, but simple)
});


// ==================== Module that handles login window logic ==================== 
// externally visible as init and processSubmit functions
// internally handles storage of username an password and OAuth token
// internally handles login first with OAuth token and then revertin to manual user&pass login
var initLogin = (function () {


    var loggedIn = false;
    // ************** Functions, public ************** 
    function init() {

        $('#btn-submit').click(processSubmit);
        $('#check').click(deviceChooser.select);

        if (localStorage.accessToken) { // try login with access token if stored
            console.log("LS AT found trying login.");
            myTryATLogin(localStorage.accessToken);
        } else { //just show UI
            if (localStorage.email) {
                console.log("LS email found trying login.");
                myTryEmailPassLogin(localStorage.email, localStorage.password);
            } else { //nothing stored, just show dialog
                console.log("Nothing found showing login.");
                $.mobile.pageContainer.pagecontainer("change", "#login", {});
            }
        }

    }

    // --- login test function ---
    function testLogin() {
        return loggedIn;
    }

    // ---------  callback for submit button --------- 
    function processSubmit() {
        var formEmail = $('#myEmail').val();
        var formPass = $('#myPassword').val();
        var logstr = "Submit pressed"; //, username:" + formEmail + " password:" + formPass;
        console.log(logstr);

        spark.login({
            username: formEmail,
            password: formPass
        }, myProcessUserPassLoginResult);
    }

    // ************** Functions, private **************
    // --------- process auth token login --------- 
    // Do not use this as is for production, account credentials stored cleartext !!!
    function myProcessATLoginResult(err, data) {
        if (err) { // if error with token, revert to username pass login, show login dialog
            loggedIn = false;
            console.log(err);
            if (localStorage.email) { //pre-load stored values to form
                $('#myEmail').val(localStorage.email); // Not safe it is to store email !!!
                $('#myPassword').val(localStorage.password); // n password to local storage. 
                spark.login({
                    username: localStorage.email,
                    password: localStorage.password
                }, myProcessUserPassLoginResult);
            }
        } else { // success with Auth token, store it
            loggedIn = true;
            if ($('#myRemember').prop('checked')) {
                console.log("Oauth token login successful!");
                localStorage.accessToken = data.accessToken; // note slight variation due ot AT login
                console.log("Storing AT");
            } else {
                localStorage.clear();
                $('#myEmail').val(''); // Not safe it is to store email
                $('#myPassword').val(''); // and password to local storage, but for proto ok. !!!
            }
        }
        $.mobile.pageContainer.pagecontainer("change", "#home", {});
        deviceChooser.init(); //after every successful login we need to select device 
    }

    //--------- process user password login--------- 
    function myProcessUserPassLoginResult(err, data) {
        $('#myLoginProgress').popup('close');
        if (err) { // if err, show error, show login dialog again
            loggedIn = false;
            console.log(err);
            setTimeout(function () {
                $('#myInvalidCredentials').popup('open');
            }, 500); // short delay needed to allow browser react
            setTimeout(function () {
                $('#myInvalidCredentials').popup('close');
            }, 3500);

        } else { // success with uname and password
            loggedIn = true;
            console.log("Username password login successful!");

            // store all values to local storage
            if ($('#myRemember').prop('checked')) {
                console.log("Storing AT, uname & pass");
                localStorage.accessToken = data.access_token; //note slight variation due to U&P login
                localStorage.email = $('#myEmail').val();
                localStorage.password = $('#myPassword').val();
            } else {
                console.log("Clearing storages!");
                localStorage.clear();
                $('#myEmail').val(''); // Clear the form too
                $('#myPassword').val(''); // 
            }
            $.mobile.pageContainer.pagecontainer("change", "#home", {});
            deviceChooser.init(); //after every successful login we need to select device 

        } // if err, revert to username pass login, show login dialog
    }

    // --------- run authentication token login --------- 
    function myTryATLogin(myAccessToken) {
        spark.login({
            accessToken: myAccessToken
        }, myProcessATLoginResult);
    }

    // --------- run authentication token login --------- 
    function myTryEmailPassLogin(email, pass) {
        $('#myEmail').attr("value", email); // Dunno if safe it is to store email
        $('#myPassword').attr("value", pass); // n password to local storage. 
        spark.login({
            username: email,
            password: pass
        }, myProcessUserPassLoginResult);
    }

    // ************** Arvertised public methods **************
    return {
        init: init,
        processSubmit: processSubmit,
        testLogin: testLogin
            //        setM : mySetMsg
    };

})();



// =======================================================================================================
// ==================== Module that handles selecting device  ==================== 

var deviceChooser = (function () {

    var myDevices;
    var managedDevice;
    // ************** Functions, public ************** 

    // try allocating same device as saved previously, or launch UI
    function init() {

        if (localStorage.managedDeviceNum) { //try set managed device
            var devicesPr = spark.listDevices(); //note: returns several when user has many
            devicesPr.then(
                function (devices) {
                    if (devices.length >= localStorage.managedDeviceNum) {
                        managedDevice = devices[localStorage.managedDeviceNum];
                        console.log("Device found from storage:", localStorage.managedDeviceNum, " Name:", managedDevice.name);
                //        myAttachSubscribers(); //not using this yet, our xtree does not send events yet
                    } else { //error conditon, saved device index not available
                        console.log("No device set, launching selector");
                        deviceChooser.select(); //imperfect solution !!!
                    }
                });
        } else { //nothing stored
            deviceChooser.select();
        }
    }



    // --------- run authentication token login --------- 
    function select() {
        var devicesPr = spark.listDevices(); //note: returns several when user has many, parsing below
        devicesPr.then(
            function (devices) {
                //                console.log('Devices: ', devices);
                $.mobile.pageContainer.pagecontainer("change", "#select", {});

                $('#myDeviceList').empty(); // simpler keep this empty than have a state 

                if (devices.length == 0) { // special case, no devices. Claiming not implemented yet.
                    alert('You do not have any devices. Please claim one: instructions at http://particle.io');
                }

                for (var i = 0; i < devices.length; i++) {
                    console.log('#:', i, ', Name: ', devices[i].name, ', Connected: ', devices[i].connected);
                    var deviceID = "deviceListId" + i;
                    var deviceIDParam = 'id="' + deviceID + '"';
                    var icon = {};
                    $('#myDeviceList').append('<li><a href="#deviceSelectedPopup" data-role="button" data-icon="check" ' + deviceIDParam + '>' + devices[i].name + '</a></li>');
                }
                deviceListBuilt = true;
                $('#myDeviceList').listview('refresh');

                $('[id^=deviceListId]').on("click", function (event, ui) { // callback for clicks
                    var thestring = $(this).attr('id');
                    var thenum = thestring.replace(/^\D+/g, ''); // find index number from id
                    managedDevice = devices[thenum]; // to define managed device
                    localStorage.managedDeviceNum = thenum;
                    console.log("Managing device: ", thenum);
        //            myAttachSubscribers(); //not used 
                    setTimeout(function () {
                        $('#deviceSelectedName').html(devices[thenum].name);
                        $('#deviceSelectedPopup').popup("open");
                    }, 100);
                    setTimeout(function () {
                        $('#deviceSelectedPopup').popup("close");
                    }, 1500);
                    setTimeout(function () {
                        $.mobile.pageContainer.pagecontainer("change", "#home", {});
                    }, 1600);
                });
            },
            function (err) {
                console.log('List devices call failed: ', err);
            }
        );
    }

    // ---------- returns currently managed device
    function getCurrentDevice() { // for external functions
        return managedDevice;
    }


    // ************** Arvertised public methods **************
    return {
        select: select,
        init: init,
        getCurrentDevice: getCurrentDevice
            //        setM : mySetMsg
    };
})();

// ====================================================================================================================
// ==================== Product logic ==================== 
// setting active program to xTree
var xTree = (function () {

    function init() { //hook up callbacks for UI buttons
        $(".program-btn").click(setProgram);
        $("#btn-sendJSON").click(sendJSON);
        
    }


    // Send arbitary JSON string to device
    function sendJSON() {
        var messageContent = $("#txt-sendJSON").val();
        console.log("SendJSON called:", messageContent);
        deviceChooser.getCurrentDevice().callFunction('setJSON', messageContent, function (err, data) {
            if (err) {
                console.log('sendJSON error:', err);
            } else {
                console.log('sendJSON called succesfully:', data);
            }
        });

    }

    
    
    
    // Set active program
    function setProgram() {
        var me = $(this);
        var messageContent = me.val();
        console.log("Set Program called:", messageContent);
        deviceChooser.getCurrentDevice().callFunction('setProgram', messageContent, function (err, data) {
            if (err) {
                console.log('setProgram error:', err);
            } else {
                console.log('setProgram called succesfully:', data);
            }
        });

    }

    
    return {
        init: init,
        setProgram: setProgram, //  sets active program
    };
})();