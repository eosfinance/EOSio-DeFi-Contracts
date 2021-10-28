ScatterJS.plugins( new ScatterEOS() );

const network = ScatterJS.Network.fromJson({
    blockchain:'eos',
    protocol:'https',
    host:'api.eossweden.org',
    port:443,
    chainId:'aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906'
});

const rpc = new eosjs_jsonrpc.default(network.fullhost());
const api = ScatterJS.eos(network, eosjs_api.default, {rpc});

const swap_account = "swap.efi";
const hubv2_account = "hub.efi";
const dopv2_account = "dop.efi";
const dmdv2_account = "dmd.efi";

// Run ScatterJS.Connect
ScatterJS.scatter.connect('SwapDAPP', {network}).then(connected => 
{
    if(!connected) return console.error('no scatter');
        ScatterJS.login().then(id=> 
        {
            if(!id) return console.error('no identity');
        })
});

window.loginScatter = () =>
{
    ScatterJS.scatter.connect('SwapDAPP', {network}).then(connected => 
    {
        if(!connected) return console.error('no scatter');
            ScatterJS.login().then(id=> 
            {
                if(!id) return console.error('no identity');
            })
    })
}

window.logoutScatter = () =>
{
    ScatterJS.scatter.logout();
}

window.maxDMD = () =>
{
    rpc.get_currency_balance('eosdmdtokens', ScatterJS.identity.accounts[0].name, "DMD").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_dmdswapamount.value = "0.00";
        else
        {
            fname_dmdswapamount.value = data[0].split(' ')[0].toString();
        }
        sel = parseInt(fname_dmdswapamount.value)
        display_calculated_bonus_dmd = sel*current_bonus/100;
        bonus_output_calculator_dmd.innerText = display_calculated_bonus_dmd;
        bonus_input_copier_dmd.innerText = fname_dmdswapamount.value+" DMD";
        }).catch(error => 
        {
            console.log(error);
        });
}

window.maxHUB = () =>
{
    rpc.get_currency_balance('eoshubtokens', ScatterJS.identity.accounts[0].name, "HUB").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_hubswapamount.value = "0.0";
        else
        {
            fname_hubswapamount.value = data[0].split(' ')[0].toString();
        }
        sel = parseInt(fname_hubswapamount.value)
        display_calculated_bonus_hub = sel*current_bonus/100;
        bonus_output_calculator_hub.innerText = display_calculated_bonus_hub;
        bonus_input_copier_hub.innerText = fname_hubswapamount.value+" HUB";
        }).catch(error => 
        {
            console.log(error);
        });
}

window.maxDOP = () =>
{
    rpc.get_currency_balance('dolphintoken', ScatterJS.identity.accounts[0].name, "DOP").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_dopswapamount.value = "0.0";
        else
        {
            fname_dopswapamount.value = data[0].split(' ')[0].toString();
        }
        sel = parseInt(fname_dopswapamount.value)
        display_calculated_bonus_dop = sel*current_bonus/100;
        bonus_output_calculator_dop.innerText = display_calculated_bonus_dop;
        bonus_input_copier_dop.innerText = fname_dopswapamount.value+" DOP";
        }).catch(error => 
        {
            console.log(error);
        });
}

// Perpetually update the status id:
const setStatus = () => 
{
    const status = document.getElementById('status');
    if (!ScatterJS)
    {
        swapCoinsNow.hidden = true;
        swap_dmd_logo.hidden = true;
        swap_hub_logo.hidden = true;
        swap_dop_logo.hidden = true;
        swap_dmd_inputform.hidden = true;
        swap_hub_inputform.hidden = true;
        swap_dop_inputform.hidden = true;
        info_tablez.hidden = true;
        usefulinfo.hidden = true;
        usefulinfoparent.hidden = true;
        yourbonus_dmd.hidden = true;
        yourbonus_hub.hidden = true;
        yourbonus_dop.hidden = true;
        todays_bonus_text.hidden = true;
        info_totalswapped_dmd.hidden = true;
        info_totalswapped_hub.hidden = true;
        info_totalswapped_dop.hidden = true;
        staking_invitation.hidden = true; // EXCEPTION

        noUsernameInfo.hidden = false;
        noUsernameInfo2.hidden = false;
        noUsernameInfo3.hidden = false;
        username.innerText = "No Scatter Detected";
        return;
    }
    if (!ScatterJS.identity) 
    {
        swapCoinsNow.hidden = true;
        swap_dmd_logo.hidden = true;
        swap_hub_logo.hidden = true;
        swap_dop_logo.hidden = true;
        swap_dmd_inputform.hidden = true;
        swap_hub_inputform.hidden = true;
        swap_dop_inputform.hidden = true;
        info_tablez.hidden = true;
        usefulinfo.hidden = true;
        usefulinfoparent.hidden = true;
        yourbonus_dmd.hidden = true;
        yourbonus_hub.hidden = true;
        yourbonus_dop.hidden = true;
        todays_bonus_text.hidden = true;
        info_totalswapped_dmd.hidden = true;
        info_totalswapped_hub.hidden = true;
        info_totalswapped_dop.hidden = true;
        staking_invitation.hidden = true; // EXCEPTION

        noUsernameInfo.hidden = false;
        noUsernameInfo2.hidden = false;
        noUsernameInfo3.hidden = false;
        username.innerText = "No identity Detected";
        return;
    }

    swapCoinsNow.hidden = false;
    swap_dmd_logo.hidden = false;
    swap_hub_logo.hidden = false;
    swap_dop_logo.hidden = false;
    swap_dmd_inputform.hidden = false;
    swap_hub_inputform.hidden = false;
    swap_dop_inputform.hidden = false;
    info_tablez.hidden = false;
    usefulinfo.hidden = false;
    usefulinfoparent.hidden = false;
    yourbonus_dmd.hidden = false;
    yourbonus_hub.hidden = false;
    yourbonus_dop.hidden = false;
    todays_bonus_text.hidden = false;
    info_totalswapped_dmd.hidden = false;
    info_totalswapped_hub.hidden = false;
    info_totalswapped_dop.hidden = false;
    staking_invitation.hidden = false; // EXCEPTION

    noUsernameInfo.hidden = true;
    noUsernameInfo2.hidden = true;
    noUsernameInfo3.hidden = true;
    username.innerText = "Username: "+ScatterJS.identity.accounts[0].name;
    usefulinfo.innerHTML = "<strong style=\"color:black\">Useful Information ("+ScatterJS.identity.accounts[0].name+")</strong>";
    /*
    Show the user his bonus in real-time by checking what is in the swap input field.
    Need to also get the current_bonus variable by looking at the contract table when it's up-and-running. 
    */
    // Let's set the bonus display and total swapped info now.

    rpc.get_table_rows({
        json: true,               // Get the response as json
        code: swap_account,       // Contract that we target
        scope: 'totals',          // Account that owns the data
        table: 'totaltable',      // Table name
        limit: 1,                 // Maximum number of rows that we want to get
        reverse: false,           // Optional: Get reversed data
        show_payer: false         // Optional: Show ram payer
    }).then(data => 
        {
            current_bonus = parseInt(data.rows[0].bonus);
            current_bonusall.innerText = current_bonus.toString() +"%";
            total_total_dmd.innerText = data.rows[0].dmd_total_swapped.split(".")[0];
            total_total_hub.innerText = data.rows[0].hub_total_swapped.split(".")[0];
            total_total_dop.innerText = data.rows[0].dop_total_swapped.split(".")[0];
        }
    ).catch(error => 
        {
            console.log(error);
        });

    rpc.get_currency_balance('eosdmdtokens', ScatterJS.identity.accounts[0].name, "DMD").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            dmd_v1_balance.innerText = "0.0000 DMD";
        else
        {
            dmd_v1_balance.innerText = data[0];
    }}).catch(error => 
        {
            console.log(error);
        });

    rpc.get_currency_balance('eoshubtokens', ScatterJS.identity.accounts[0].name, "HUB").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            hub_v1_balance.innerText = "0.0000 HUB";
        else
        {
            hub_v1_balance.innerText = data[0];
    }}).catch(error => 
        {
            console.log(error);
        });

    rpc.get_currency_balance('dolphintoken', ScatterJS.identity.accounts[0].name, "DOP").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            dop_v1_balance.innerText = "0.0000 DOP";
        else
        {
            dop_v1_balance.innerText = data[0];
    }}).catch(error => 
        {
            console.log(error);
        });
    // V2 token info:
    rpc.get_currency_balance(dmdv2_account, ScatterJS.identity.accounts[0].name, "DMD").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            dmd_v2_balance.innerText = "0.0000 DMD";
        else
        {
            dmd_v2_balance.innerText = data[0];
    }}).catch(error => 
        {
            console.log(error);
        });

    rpc.get_currency_balance(hubv2_account, ScatterJS.identity.accounts[0].name, "HUB").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            hub_v2_balance.innerText = "0.0000 HUB";
        else
        {
            hub_v2_balance.innerText = data[0];
    }}).catch(error => 
        {
            console.log(error);
        });

    rpc.get_currency_balance(dopv2_account, ScatterJS.identity.accounts[0].name, "DOP").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            dop_v2_balance.innerText = "0.0000 DOP";
        else
        {
            dop_v2_balance.innerText = data[0];
    }}).catch(error => 
        {
            console.log(error);
        });

    // Player's "total swapped" info:
    rpc.get_table_rows({
        json: true,               // Get the response as json
        code: 'swap.efi',       // Contract that we target
        scope: 'swap.efi',      // Account that owns the data
        table: 'swaptable',       // Table name
        limit: 500,               // Maximum number of rows that we want to get
        reverse: false,           // Optional: Get reversed data
        show_payer: false         // Optional: Show ram payer
    }).then(data => 
    {
        for (let i = 0; i <= 500; i++)
        {
            //console.log(data.rows[0].owner_account);
            if (data.rows[i].owner_account == ScatterJS.identity.accounts[0].name)
            {
                if (data.rows[i].dmd_swapped_amount.length > 2)
                    {
                        dmd_total_swapped.innerText = data.rows[i].dmd_swapped_amount;
                    }
                else
                    dmd_total_swapped.innerText = "0.0000 DMD";
                /*             */ /*             */ /*             */
                 if (data.rows[i].hub_swapped_amount.length > 2)
                    {
                        hub_total_swapped.innerText = data.rows[i].hub_swapped_amount;
                    }
                else
                    hub_total_swapped.innerText = "0.0000 HUB";
                /*             */ /*             */ /*             */
                 if (data.rows[i].dop_swapped_amount.length > 2)
                    {
                        dop_total_swapped.innerText = data.rows[i].dop_swapped_amount;
                    }
                else
                    dop_total_swapped.innerText = "0.0000 DOP";

                break;
            }
        }
    }).catch(error => 
        {
            console.log(error);
        });
}
    //console.log(swaptable_response);


const formatTokensHUB = (input) =>
{
    if (input.includes(".") == true)
    {   // If there's a dot present in the user input, look to see if there's 4 decimals, if not, add zeroes until there are.
        // If there's more than 4 decimals, we truncate the value to 4 decimals.
        decimals = input.split('.')[1]
        wholenumbers = input.split('.')[0]
        if (decimals.length == 4)
        {
            return input
        }
        if (decimals.length < 4)
        {
            for (let i = decimals.length; i < 4; i++)
            {
                decimals += "0"
            }
            return wholenumbers+"."+decimals
        }
        if (decimals.length > 4)
        {
            decimals = decimals.substring(0,4);
            return wholenumbers+"."+decimals
        }
    }
    else
    {
        return input+".0000"
    }
}

const formatTokensDOP = (input) =>
{
    if (input.includes(".") == true)
    {   // If there's a dot present in the user input, look to see if there's 4 decimals, if not, add zeroes until there are.
        // If there's more than 4 decimals, we truncate the value to 4 decimals.
        decimals = input.split('.')[1]
        wholenumbers = input.split('.')[0]
        if (decimals.length == 8)
        {
            return input
        }
        if (decimals.length < 8)
        {
            for (let i = decimals.length; i < 4; i++)
            {
                decimals += "0"
            }
            return wholenumbers+"."+decimals
        }
        if (decimals.length > 8)
        {
            decimals = decimals.substring(0,8);
            return wholenumbers+"."+decimals
        }
    }
    else
    {
        return input+".00000000"
    }
}

const formatTokensDMD = (input) =>
{
    if (input.includes(".") == true)
    {   // If there's a dot present in the user input, look to see if there's 4 decimals, if not, add zeroes until there are.
        // If there's more than 4 decimals, we truncate the value to 4 decimals.
        decimals = input.split('.')[1]
        wholenumbers = input.split('.')[0]
        if (decimals.length == 10)
        {
            return input
        }
        if (decimals.length < 10)
        {
            for (let i = decimals.length; i < 10; i++)
            {
                decimals += "0"
            }
            return wholenumbers+"."+decimals
        }
        if (decimals.length > 10)
        {
            decimals = decimals.substring(0,10);
            return wholenumbers+"."+decimals
        }
    }
    else
    {
        return input+".0000000000"
    }
}

window.swapHUB = () => 
{ 
    if ((bonus_input_copier_hub.innerText == "0 HUB") || (fname_hubswapamount.value == ""))
    {
        return;
    }

    tokens = bonus_input_copier_hub.innerText.split(' ')[0];
    swap_quantity = formatTokensHUB(tokens)+" HUB";

    console.log("Swap Quantity has been set to:", swap_quantity);

    api.transact({
                    actions:[{
                        account: 'eoshubtokens',
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: swap_account,
                            quantity: swap_quantity,
                            memo: 'I am swapping my old V1 HUB for the new V2 HUB!',
                        }
                    }]
                }, {
                    blocksBehind: 3,
                    expireSeconds: 30,
                }).then(res => {
                    console.log('sent tx: ', res);
                }).catch(err => {
                    alert(err);
                });
    bonus_input_copier_hub.innerText = "0 HUB";
    bonus_output_calculator_hub.innerText = "0";
    fname_hubswapamount.value = "";
}

window.swapDOP = () =>
{ 
    if ((bonus_input_copier_dop.innerText == "0 DOP") || (fname_dopswapamount.value == ""))
    {
        return;
    }

    tokens = bonus_input_copier_dop.innerText.split(' ')[0];
    swap_quantity = formatTokensDOP(tokens)+" DOP";

    console.log("Swap Quantity has been set to:", swap_quantity);

    api.transact({
                    actions:[{
                        account: 'dolphintoken',
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: swap_account,
                            quantity: swap_quantity,
                            memo: 'I am swapping my old V1 DOP for the new V2 DOP!',
                        }
                    }]
                }, {
                    blocksBehind: 3,
                    expireSeconds: 30,
                }).then(res => {
                    console.log('sent tx: ', res);
                }).catch(err => {
                    alert(err);
                });
    bonus_input_copier_dop.innerText = "0 DOP";
    bonus_output_calculator_dop.innerText = "0";
    fname_dopswapamount.value = "";
}

window.swapDMD = () =>
{ 
    if ((bonus_input_copier_dmd.innerText == "0 DMD") || (fname_dmdswapamount.value == ""))
    {
        return;
    }
    tokens = bonus_input_copier_dmd.innerText.split(' ')[0];
    swap_quantity = formatTokensDMD(tokens)+" DMD";

    console.log("Swap Quantity has been set to:", swap_quantity);

    api.transact({
                    actions:[{
                        account: 'eosdmdtokens',
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: swap_account,
                            quantity: swap_quantity,
                            memo: 'I am swapping my old V1 DMD for the new V2 DMD!',
                        }
                    }]
                }, {
                    blocksBehind: 3,
                    expireSeconds: 30,
                }).then(res => {
                    console.log('sent tx: ', res);
                }).catch(err => {
                    alert(err);
                });
    bonus_input_copier_dmd.innerText = "0 DMD";
    bonus_output_calculator_dmd.innerText = "0";
    fname_dmdswapamount.value = "";
}


window.swapInputKeyPress = () =>
{
    if ((document.getElementById("fname_dmdswapamount").value) != "")
        bonus_input_copier_dmd.innerText = document.getElementById("fname_dmdswapamount").value+" DMD";
    else
        bonus_input_copier_dmd.innerText = "0 DMD";

    if ((document.getElementById("fname_hubswapamount").value) != "")
        bonus_input_copier_hub.innerText = document.getElementById("fname_hubswapamount").value+" HUB";
    else
        bonus_input_copier_hub.innerText = "0 HUB";

    if ((document.getElementById("fname_dopswapamount").value) != "")
        bonus_input_copier_dop.innerText = document.getElementById("fname_dopswapamount").value+" DOP";
    else
        bonus_input_copier_dop.innerText = "0 DOP";

    selected_dmd = parseInt(document.getElementById("fname_dmdswapamount").value.split(' ')[0]);
    selected_hub = parseInt(document.getElementById("fname_hubswapamount").value.split(' ')[0]);
    selected_dop = parseInt(document.getElementById("fname_dopswapamount").value.split(' ')[0]);

    if (Number.isInteger(selected_dmd)){
        display_calculated_bonus_dmd = selected_dmd*current_bonus/100;
        bonus_output_calculator_dmd.innerText = display_calculated_bonus_dmd;}
    else
        bonus_output_calculator_dmd.innerText = "0"

    if (Number.isInteger(selected_hub)){
        display_calculated_bonus_hub = selected_hub*current_bonus/100;
        bonus_output_calculator_hub.innerText = display_calculated_bonus_hub;}
    else
        bonus_output_calculator_hub.innerText = "0"

    if (Number.isInteger(selected_dop)){
        display_calculated_bonus_dop = selected_dop*current_bonus/100;
        bonus_output_calculator_dop.innerText = display_calculated_bonus_dop;}
    else
        bonus_output_calculator_dop.innerText = "0"
}
setStatus();
setInterval(() =>
{
    setStatus();
}, 4000);
