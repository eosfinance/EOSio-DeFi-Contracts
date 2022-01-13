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

const nft_account = "nft.efi";


ScatterJS.scatter.connect('NFTDAPP', {network}).then(connected => 
{
    if(!connected) return console.error('no scatter');
        ScatterJS.login().then(id=> 
        {
            if(!id) return console.error('no identity');
        })
});

window.loginScatter = () =>
{
    ScatterJS.scatter.connect('NFTDAPP', {network}).then(connected => 
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

const setStatus = () => 
{
    const status = document.getElementById('status');
    if (!ScatterJS)
    {
        username.innerText = "No Scatter Detected";
        return;
    }
    if (!ScatterJS.identity) 
    {
        username.innerText = "No identity Detected";
        return;
    }
    username.innerText = "Username: "+ScatterJS.identity.accounts[0].name;
    usefulinfo.innerHTML = "<strong style=\"color:black\">Useful Information ("+ScatterJS.identity.accounts[0].name+")</strong>";

    // Settings the user's EOS Balance in the useful info:
    rpc.get_currency_balance('eosio.token', ScatterJS.identity.accounts[0].name, "EOS").then(data => 
    {
        if (data[0] === undefined || data[0].length == 0)
            eos_balance.innerHTML = '0.0000 <strong>EOS <img style="vertical-align:middle" src="/wp-content/uploads/2021/08/eoslogo_30x30.png"</img></strong>';
        else
        {
            eos_balance.innerHTML = data[0].split(' ')[0].split('.')[0];
            eos_balance.innerHTML += '<strong> EOS <img style="vertical-align:middle" src="/wp-content/uploads/2021/08/eoslogo_30x30.png"</img></strong>';
        }
        }).catch(error => 
        {
            console.log(error);
        });
    // Let's now set the user's Golden NFT balances:
    rpc.get_table_rows({
        json: true,                  // Get the response as json
        code: nft_account,           // Contract that we target
        scope: nft_account,          // Account that owns the data
        table: 'countable',          // Table name
        limit: 550,                  // Maximum number of rows that we want to get
        reverse: false,              // Optional: Get reversed data
        show_payer: false            // Optional: Show ram payer
    }).then(data => 
    {
        for (let i = 0; i <= 550; i++)
        {
            //console.log(data.rows[0].owner_account);
            if (data.rows[i].owner_account == ScatterJS.identity.accounts[0].name)
            {

                golden_dmd_balance.innerText = data.rows[i].total_dmd;
                golden_hub_balance.innerText = data.rows[i].total_hub;
                golden_dop_balance.innerText = data.rows[i].total_dop;
                break;
            }
        }
    }).catch(error => 
        {
            console.log(error);
        });

    // Now let's set the display for the total minted:
    rpc.get_table_rows({
        json: true,                  // Get the response as json
        code: nft_account,           // Contract that we target
        scope: "totals",             // Account that owns the data
        table: 'totaltable',         // Table name
        limit: 3,                    // Maximum number of rows that we want to get
        reverse: false,              // Optional: Get reversed data
        show_payer: false            // Optional: Show ram payer
    }).then(data => 
    {
        dmd_total_minted.innerText = data.rows[0].dmd_total_minted;
        dop_total_minted.innerText = data.rows[0].dop_total_minted;
        hub_total_minted.innerText = data.rows[0].hub_total_minted;
    }).catch(error => 
        {
            console.log(error);
        });

}
    
window.swapInputKeyPress = () =>
{   // dop <----------------------
    if ((document.getElementById("dop_mint_amount").value) != "")
    {
        dop_mint_copier.innerText = document.getElementById("dop_mint_amount").value;
        if (parseInt(document.getElementById("dop_mint_amount").value) > 1)
            dop_plural_singular.innerText = "Dolphins"
        else
            dop_plural_singular.innerText = "Dolphin"

        dop_mint_amount_multiplier.innerText = parseInt(document.getElementById("dop_mint_amount").value) *27;
    }
    else
    {
        dop_mint_copier.innerText = "1";
    }
    // hub <----------------------
    if ((document.getElementById("hub_mint_amount").value) != "")
    {
        hub_mint_copier.innerText = document.getElementById("hub_mint_amount").value;
        if (parseInt(document.getElementById("hub_mint_amount").value) > 1)
            ehub_plural_singular.innerText = "eHUBs"
        else
            ehub_plural_singular.innerText = "eHUB"

        hub_mint_amount_multiplier.innerText = parseInt(document.getElementById("hub_mint_amount").value) *27;
    }
    else
    {
        hub_mint_copier.innerText = "1";
    }
    // dmd <----------------------
    if ((document.getElementById("dmd_mint_amount").value) != "")
    {
        dmd_mint_copier.innerText = document.getElementById("dmd_mint_amount").value;
        if (parseInt(document.getElementById("dmd_mint_amount").value) > 1)
            dmd_plural_singular.innerText = "Diamonds"
        else
            dmd_plural_singular.innerText = "Diamond"

        dmd_mint_amount_multiplier.innerText = parseInt(document.getElementById("dmd_mint_amount").value) *27;
    }
    else
    {
        dmd_mint_copier.innerText = "1";
    }

}


window.mintHUB = () => 
{ 
    int_eos_to_send = parseInt(document.getElementById("hub_mint_amount").value) *27
    eos_to_send = int_eos_to_send.toString() + ".0000 EOS"

    console.log("eos_to_send:", eos_to_send);

    api = ScatterJS.eos(network, eosjs_api.default, {rpc});

    api.transact({
                    actions:[{
                        account: "eosio.token",
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: nft_account,
                            quantity: eos_to_send,
                            memo: 'hub',
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

window.mintDOP = () => 
{ 
    int_eos_to_send = parseInt(document.getElementById("dop_mint_amount").value) *27
    eos_to_send = int_eos_to_send.toString() + ".0000 EOS"

    console.log("eos_to_send:", eos_to_send);

    api = ScatterJS.eos(network, eosjs_api.default, {rpc});

    api.transact({
                    actions:[{
                        account: "eosio.token",
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: nft_account,
                            quantity: eos_to_send,
                            memo: 'dop',
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

window.mintDMD = () => 
{ 
    int_eos_to_send = parseInt(document.getElementById("dmd_mint_amount").value) *27
    eos_to_send = int_eos_to_send.toString() + ".0000 EOS"

    console.log("eos_to_send:", eos_to_send);

    api = ScatterJS.eos(network, eosjs_api.default, {rpc});

    api.transact({
                    actions:[{
                        account: "eosio.token",
                        name: 'transfer',
                        authorization: // user paying for resources must go first
                        [{
                            actor: ScatterJS.identity.accounts[0].name,
                            permission: ScatterJS.identity.accounts[0].authority,
                        }],
                        data: 
                        {
                            from: ScatterJS.identity.accounts[0].name,
                            to: nft_account,
                            quantity: eos_to_send,
                            memo: 'dmd',
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

setStatus();
setInterval(() =>
{
    setStatus();
}, 5000);
