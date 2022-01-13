ScatterJS.plugins( new ScatterEOS() );

const network = ScatterJS.Network.fromJson({
    blockchain:'eos',
    protocol:'https',
    host:'eos.greymass.com',
    //host:'eospush.tokenpocket.pro',
    port:443,
    chainId:'aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906'
});

let api;
const rpc = new eosjs_jsonrpc.default(network.fullhost());
//const api = ScatterJS.eos(network, eosjs_api.default, {rpc});

const stake_account = "stake.efi";
const hubv2_account = "hub.efi";
const dopv2_account = "dop.efi";
const dmdv2_account = "dmd.efi";

// Run ScatterJS.Connect
ScatterJS.scatter.connect('StakeDAPP', {network}).then(connected => 
{
    if(!connected) return console.error('no scatter');
        ScatterJS.login().then(id=> 
        {
            if(!id) return console.error('no identity');
        })
});

window.loginScatter = () =>
{
    ScatterJS.scatter.connect('StakeDAPP', {network}).then(connected => 
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
    rpc.get_currency_balance(dmdv2_account, ScatterJS.identity.accounts[0].name, "DMD").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_dmdstakeamount.value = "0.00";
        else
        {
            fname_dmdstakeamount.value = (parseFloat(data[0].split(' ')[0]) *100/100).toString();
        }
        }).catch(error => 
        {
            console.log(error);
        });
}

window.maxHUB = () =>
{
    rpc.get_currency_balance(hubv2_account, ScatterJS.identity.accounts[0].name, "HUB").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_hubstakeamount.value = "0.0";
        else
        {
            fname_hubstakeamount.value = (parseFloat(data[0].split(' ')[0]) *100/100).toString();
        }
        }).catch(error => 
        {
            console.log(error);
        });
}

window.maxDOP = () =>
{
    rpc.get_currency_balance(dopv2_account, ScatterJS.identity.accounts[0].name, "DOP").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            fname_dopstakeamount.value = "0.0";
        else
        {
            fname_dopstakeamount.value = (parseFloat(data[0].split(' ')[0]) *100/100).toString();
        }
        }).catch(error => 
        {
            console.log(error);
        });
}

window.stakeHUB = () => 
{ 
    //console.log("fname_hubstakeamount.innerText",fname_hubstakeamount.value);
    if (fname_hubstakeamount.value == "")
    {
        //console.log("fname_hubstakeamount is zero");
        return;
    }

    tokens = fname_hubstakeamount.value;
    stake_quantity = formatTokens(tokens)+" HUB";

    //console.log("Stake Quantity has been set to:", stake_quantity);
    api = ScatterJS.eos(network, eosjs_api.default, {rpc});

    api.transact({
                    actions:[{
                        account: hubv2_account,
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: stake_account,
                            quantity: stake_quantity,
                            memo: 'I am staking my HUB in the special EFi V2 promotion!',
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
    fname_hubstakeamount.value = ""; 
}

window.stakeDOP = () => 
{ 
    //console.log("fname_dopstakeamount.innerText",fname_dopstakeamount.value);
    if (fname_dopstakeamount.value == "")
    {
        //console.log("fname_dopstakeamount is zero");
        return;
    }

    tokens = fname_dopstakeamount.value;
    stake_quantity = formatTokens(tokens)+" DOP";

    //console.log("Stake Quantity has been set to:", stake_quantity);
    api = ScatterJS.eos(network, eosjs_api.default, {rpc});

    api.transact({
                    actions:[{
                        account: dopv2_account,
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: stake_account,
                            quantity: stake_quantity,
                            memo: 'I am staking my DOP in the special EFi V2 promotion!',
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
    fname_dopstakeamount.value = ""; 
}

window.stakeDMD = () => 
{ 
    //console.log("fname_dmdstakeamount.innerText",fname_dmdstakeamount.value);
    if (fname_dmdstakeamount.value == "")
    {
        //console.log("fname_dmdstakeamount is zero");
        return;
    }

    tokens = fname_dmdstakeamount.value;
    stake_quantity = formatTokens(tokens)+" DMD";

    //console.log("Stake Quantity has been set to:", stake_quantity);
    api = ScatterJS.eos(network, eosjs_api.default, {rpc});

    api.transact({
                    actions:[{
                        account: dmdv2_account,
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: stake_account,
                            quantity: stake_quantity,
                            memo: 'I am staking my DMD in the special EFi V2 promotion!',
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
    fname_dmdstakeamount.value = ""; 
}

window.claimDMD = () =>
{
    api = ScatterJS.eos(network, eosjs_api.default, {rpc});
    api.transact({
                    actions:[{
                        account: stake_account,
                        name: 'claimdmd',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            owner_account: ScatterJS.identity.accounts[0].name,
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
}


window.claimHUB = () =>
{
    api = ScatterJS.eos(network, eosjs_api.default, {rpc});
    api.transact({
                    actions:[{
                        account: stake_account,
                        name: 'claimhub',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            owner_account: ScatterJS.identity.accounts[0].name,
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
}

window.claimDOP = () =>
{
    api = ScatterJS.eos(network, eosjs_api.default, {rpc});
    api.transact({
                    actions:[{
                        account: stake_account,
                        name: 'claimdop',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            owner_account: ScatterJS.identity.accounts[0].name,
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
}


window.withdraw = () =>
{
    api = ScatterJS.eos(network, eosjs_api.default, {rpc});
    api.transact({
                    actions:[{
                        account: stake_account,
                        name: 'withdraw',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            owner_account: ScatterJS.identity.accounts[0].name,
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
}

const formatTokens = (input) => // We don't need three separate formatTokens functions in the staking page because they all have precision 4.
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

// Perpetually update the status id
const setStatus = () => 
{
    const status = document.getElementById('status');
    if (!ScatterJS)
    {
        v2_staking_title.hidden = true;
        stake_dmd_logo.hidden = true;
        stake_hub_logo.hidden = true;
        stake_dop_logo.hidden = true;
        stake_dmd_inputform.hidden = true;
        stake_hub_inputform.hidden = true;
        stake_dop_inputform.hidden = true;
        info_table.hidden = true;
        usefulinfo.hidden = true;
        usefulinfoparent.hidden = true;
        swap_invitation.hidden = true;
        unclaimed_dmd_readonly.hidden = true;
        unclaimed_hub_readonly.hidden = true;
        unclaimed_dop_readonly.hidden = true;
        hub_apr_parent.hidden = true;
        dop_apr_parent.hidden = true;
        dmd_apr_parent.hidden = true;
        importantwarning.hidden = true;
        total_dop_staked_parent.hidden = true;
        total_dmd_staked_parent.hidden = true;
        total_hub_staked_parent.hidden = true;
        withdrawcoins.hidden = true;

        username.innerText = "No Scatter Detected";
        return;
    }
    if (!ScatterJS.identity) 
    {
        v2_staking_title.hidden = true;
        stake_dmd_logo.hidden = true;
        stake_hub_logo.hidden = true;
        stake_dop_logo.hidden = true;
        stake_dmd_inputform.hidden = true;
        stake_hub_inputform.hidden = true;
        stake_dop_inputform.hidden = true;
        info_table.hidden = true;
        usefulinfo.hidden = true;
        usefulinfoparent.hidden = true;
        swap_invitation.hidden = true;
        unclaimed_dmd_readonly.hidden = true;
        unclaimed_hub_readonly.hidden = true;
        unclaimed_dop_readonly.hidden = true;
        hub_apr_parent.hidden = true;
        dop_apr_parent.hidden = true;
        dmd_apr_parent.hidden = true;
        importantwarning.hidden = true;
        total_dop_staked_parent.hidden = true;
        total_dmd_staked_parent.hidden = true;
        total_hub_staked_parent.hidden = true;
        withdrawcoins.hidden = true;

        username.innerText = "No identity Detected";
        return;
    }

    v2_staking_title.hidden = false;
    stake_dmd_logo.hidden = false;
    stake_hub_logo.hidden = false;
    stake_dop_logo.hidden = false;
    stake_dmd_inputform.hidden = false;
    stake_hub_inputform.hidden = false;
    stake_dop_inputform.hidden = false;
    info_table.hidden = false;
    usefulinfo.hidden = false;
    usefulinfoparent.hidden = false;
    swap_invitation.hidden = false;
    unclaimed_dmd_readonly.hidden = false;
    unclaimed_hub_readonly.hidden = false;
    unclaimed_dop_readonly.hidden = false;
    hub_apr_parent.hidden = true; // Made these hidden after the Staking ended.
    dop_apr_parent.hidden = true; // Made these hidden after the Staking ended.
    dmd_apr_parent.hidden = true; // Made these hidden after the Staking ended.
    importantwarning.hidden = false;
    total_dop_staked_parent.hidden = false;
    total_dmd_staked_parent.hidden = false;
    total_hub_staked_parent.hidden = false;
    withdrawcoins.hidden = false;

    username.innerText = "Username: "+ScatterJS.identity.accounts[0].name;
    usefulinfo.innerHTML = "<strong style=\"color:black\">Useful Information ("+ScatterJS.identity.accounts[0].name+")</strong>";

    rpc.get_currency_balance('eosdmdtokens', ScatterJS.identity.accounts[0].name, "DMD").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            dmd_v1_balance.innerText = "0.0000 DMD";
        else
        {
            dmd_v1_balance.innerText = data[0];
            //console.log(data[0]);
        }
        }).catch(error => 
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
            //console.log(data[0]);
        }
        }).catch(error => 
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
            //console.log(data[0]);
        }
        }).catch(error => 
        {
            console.log(error);
        });

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

    // Get the APR:

    rpc.get_table_rows({
        json: true,                  // Get the response as json
        code: stake_account,         // Contract that we target
        scope: "totals",             // Account that owns the data
        table: 'totaltable',         // Table name
        limit: 3,                    // Maximum number of rows that we want to get
        reverse: false,              // Optional: Get reversed data
        show_payer: false            // Optional: Show ram payer
    }).then(data => 
    {
        // First we get the issue rate:
        hub_issue_frequency = data.rows[0].hub_issue_frequency;
        dop_issue_frequency = data.rows[0].dop_issue_frequency;
        dmd_issue_frequency = data.rows[0].dmd_issue_frequency;
        // Then we get the total staked:
        hub_total_staked = data.rows[0].hub_total_staked;
        dop_total_staked = data.rows[0].dop_total_staked;
        dmd_total_staked = data.rows[0].dmd_total_staked;

        // Filling in the website info with total staked:
        total_hub_staked.innerText = String(parseInt(hub_total_staked.split(' ')[0]))+" HUB";
        total_dop_staked.innerText = String(parseInt(dop_total_staked.split(' ')[0]))+" DOP";
        total_dmd_staked.innerText = String(parseInt(dmd_total_staked.split(' ')[0]))+" DMD";

        hub_rate_per_sec = hub_issue_frequency/10000;
        dop_rate_per_sec = dop_issue_frequency/10000;
        dmd_rate_per_sec = dmd_issue_frequency/10000;
        //console.log("hub_rate_per_sec ",hub_rate_per_sec);

        hubs_released_per_year = hub_rate_per_sec * 31536000; // seconds in a year
        dops_released_per_year = dop_rate_per_sec * 31536000;
        dmds_released_per_year = dmd_rate_per_sec * 31536000;
        //console.log("hubs_released_per_year:",hubs_released_per_year);

        calc_hub_apr = hubs_released_per_year/parseInt(hub_total_staked.split(' ')[0])*100;
        calc_dop_apr = dops_released_per_year/parseInt(dop_total_staked.split(' ')[0])*100;
        calc_dmd_apr = dmds_released_per_year/parseInt(dmd_total_staked.split(' ')[0])*100;

        hub_apr.innerText = parseInt(calc_hub_apr);
        dop_apr.innerText = parseInt(calc_dop_apr);
        dmd_apr.innerText = parseInt(calc_dmd_apr);

        // We know how many are released each second, and we know how many of each tokens are staked in total.
        // We can now calculate the reward over a whole year, and see how much that is, percentage wise, relative to the total amount staked for each coin.
    }).catch(error => 
        {
            console.log(error);
        });

    // Player's "total staked" info:
    rpc.get_table_rows({
        json: true,                  // Get the response as json
        code: stake_account,         // Contract that we target
        scope: stake_account,        // Account that owns the data
        table: 'staketable',         // Table name
        limit: 500,                  // Maximum number of rows that we want to get
        reverse: false,              // Optional: Get reversed data
        show_payer: false            // Optional: Show ram payer
    }).then(data => 
    {
        for (let i = 0; i <= 500; i++)
        {
            //console.log(data.rows[0].owner_account);
            if (data.rows[i].owner_account == ScatterJS.identity.accounts[0].name)
            {
                // Staked coins info:
                if (data.rows[i].dmd_staked_amount.length > 2)
                    {
                        display_dmd_total_staked.innerText = data.rows[i].dmd_staked_amount;
                    }
                else
                    display_dmd_total_staked.innerText = "0.0000 DMD";
                /*             */ /*             */ /*             */
                 if (data.rows[i].hub_staked_amount.length > 2)
                    {
                        display_hub_total_staked.innerText = data.rows[i].hub_staked_amount;
                    }
                else
                    display_hub_total_staked.innerText = "0.0000 HUB";
                /*             */ /*             */ /*             */
                 if (data.rows[i].dop_staked_amount.length > 2)
                    {
                        display_dop_total_staked.innerText = data.rows[i].dop_staked_amount;
                    }
                else
                    display_dop_total_staked.innerText = "0.0000 DOP";


                // Claimed coins info:
                if (data.rows[i].dmd_claimed_amount.length > 2)
                    {
                        dmd_total_claimed.innerText = data.rows[i].dmd_claimed_amount;
                    }
                else
                    dmd_total_claimed.innerText = "0.0000 DMD";
                /*             */ /*             */ /*             */
                 if (data.rows[i].hub_claimed_amount.length > 2)
                    {
                        hub_total_claimed.innerText = data.rows[i].hub_claimed_amount;
                    }
                else
                    hub_total_claimed.innerText = "0.0000 HUB";
                /*             */ /*             */ /*             */
                 if (data.rows[i].dop_claimed_amount.length > 2)
                    {
                        dop_total_claimed.innerText = data.rows[i].dop_claimed_amount;
                    }
                else
                    dop_total_claimed.innerText = "0.0000 DOP";


                // Let's update the user's "Unclaimed Amount" field at the bottom now:
                if (data.rows[i].dmd_unclaimed_amount.length > 2)
                    {
                        unclaimed_dmd_readonly_display.value = data.rows[i].dmd_unclaimed_amount.split(" ")[0];
                    }
                else
                    unclaimed_dmd_readonly_display.value = "0.0000";
                /*             */ /*             */ /*             */
                 if (data.rows[i].hub_unclaimed_amount.length > 2)
                    {
                        unclaimed_hub_readonly_display.value = data.rows[i].hub_unclaimed_amount.split(" ")[0];
                    }
                else
                    unclaimed_hub_readonly_display.value = "0.0000";
                /*             */ /*             */ /*             */
                 if (data.rows[i].dop_unclaimed_amount.length > 2)
                    {
                        unclaimed_dop_readonly_display.value = data.rows[i].dop_unclaimed_amount.split(" ")[0];
                    }
                else
                    unclaimed_dop_readonly_display.value = "0.0000";

                break;
            }
        }
    }).catch(error => 
        {
            console.log(error);
        });
}

setStatus();
setInterval(() =>
{
    setStatus();
}, 5000);
