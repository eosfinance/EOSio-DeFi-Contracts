#include <nft.hpp>
/*
This is the EFi V2 NFT Contract.
*/

void nftcontrak::set()
{  // We will initiate the three tables here, and then that's basically all we gotta do.
    require_auth(get_self());

    totaltable total_minted(get_self(), "totals"_n.value); 
    auto total_it = total_minted.find("totals"_n.value);
    if(total_it == total_minted.end())
    {   // Create the "totals" row in the totals table and init it with zero for each column.
        total_minted.emplace(get_self(), [&](auto& row) 
        {
            row.key = "totals"_n;
            row.hub_total_minted = 0;
            row.dop_total_minted = 0;
            row.dmd_total_minted = 0;
        });
    }
    
    countable buyers_total(get_self(), get_self().value);
    auto buyers_it = buyers_total.find(get_self().value);
    if(buyers_it == buyers_total.end())
    {
        buyers_total.emplace(get_self(), [&](auto& row) 
        {
            row.owner_account = get_self();
            row.total_hub     = 0;
            row.total_dop     = 0;
            row.total_dmd     = 0;
        });
    }

    mintingtable nft_mints(get_self(), get_self().value);
    auto mints_it = nft_mints.find(get_self().value);
    if(mints_it == nft_mints.end())
    {
        nft_mints.emplace(get_self(), [&](auto& row) 
        {
            row.mint_order    = 0;
            row.owner_account = get_self();
            row.amount_hub    = 0;
            row.amount_dop    = 0;
            row.amount_dmd    = 0;
        });
    }
}

[[eosio::on_notify("eosio.token::transfer")]]
void nftcontrak::registernft(const name& owner_account, const name& to, const asset& amount_eos_sent, std::string memo)
{
    check(amount_eos_sent.symbol == eos_symbol, "error: these are not the droids you are looking for.");
    // Checks how much EOS has been sent and what the memo is.
    // Registers the user in the buyers and mints tables. Updates the total table.
    if (to != get_self() || owner_account == get_self())
    {
        print("error: these are not the droids you are looking for.");
        return;
    }

    uint64_t nft_price = 190000; // 19 EOS
    bool incorrectmemo = false;
    if ( (memo != "hub") && (memo != "dop") && (memo != "dmd") )
    {
        incorrectmemo = true;
    }
    check(!incorrectmemo,"error: you must specify the correct memo: `hub` or `dop` or `dmd`");
    check(amount_eos_sent.amount >= nft_price, "error: you must send at least 19 EOS to mint a Golden NFT.");

    totaltable total_minted(get_self(), "totals"_n.value);
    auto total_it = total_minted.find("totals"_n.value);
    check(total_it != total_minted.end(), "error: totals table is not initiated."); 
    // Here we check to see if the user wants to buy extra NFTs, over the 100 that is allowed.
    bool overflow = false;
    uint16_t limit = 137;

    // Do a check to see if we've already reached over the limit.
    // The only way for a user to buy over the 137 limit, is if the counter is at 135 or something, and they buy more than 2.
    // In that case, we will just manually refund the users.
    if ( (memo == "hub" && total_it->hub_total_minted >=limit) || 
           (memo == "dop" && total_it->dop_total_minted >=limit) || 
             (memo == "dmd" && total_it->dmd_total_minted >=limit) )
    {
        overflow = true;
    }

    check(!overflow, "error: the maximum number for this NFT has been reached (137 have been minted)");
    // Let's calculate how many NFTs the user has bought at this point, by looking at how much EOS they've sent.
    eosio::print_f("User has sent: [%] EOS \n",amount_eos_sent.amount);
    eosio::print_f("NFT Price: [%]\n",nft_price);

    uint16_t minted_nfts = amount_eos_sent.amount/nft_price;
    eosio::print_f("Number of minted NFTs: [%]\n",minted_nfts);
    uint16_t hub_minted = 0;
    uint16_t dop_minted = 0;
    uint16_t dmd_minted = 0;

    if (memo == "hub")
        hub_minted = minted_nfts;
    else if (memo == "dop")
        dop_minted = minted_nfts;
    else
        dmd_minted = minted_nfts;

    // Update the three tables:
    total_minted.modify(total_it, get_self(), [&](auto& row) 
    {
        row.hub_total_minted += hub_minted;
        row.dop_total_minted += dop_minted;
        row.dmd_total_minted += dmd_minted;
    });
    
    countable buyers_total(get_self(), get_self().value);
    auto buyers_it = buyers_total.find(owner_account.value);
    if(buyers_it == buyers_total.end())
    {
        buyers_total.emplace(get_self(), [&](auto& row)
        {
            row.owner_account = owner_account;
            row.total_hub     = hub_minted;
            row.total_dop     = dop_minted;
            row.total_dmd     = dmd_minted;
        });
    }
    else
    {
        buyers_total.modify(buyers_it, get_self(), [&](auto& row)
        {
            row.total_hub += hub_minted;
            row.total_dop += dop_minted;
            row.total_dmd += dmd_minted;
        });
    }

    mintingtable nft_mints(get_self(), get_self().value);
    auto begin_itr = nft_mints.begin();
    auto end_itr = nft_mints.end();
    uint16_t counter = 0;

    while (begin_itr != end_itr)
    {
        counter++;
        begin_itr++;
    }
    eosio::print_f("nft_mints table row count is at: [%]\n",counter);
    nft_mints.emplace(get_self(), [&](auto& row) 
    {
        row.mint_order    = counter;
        row.owner_account = owner_account;
        row.amount_hub    = hub_minted;
        row.amount_dop    = dop_minted;
        row.amount_dmd    = dmd_minted;
    });

    uint32_t refund_amount = amount_eos_sent.amount - minted_nfts*nft_price;

    eosio::print_f("Refund for user: [%]\n",refund_amount);
    if (refund_amount > 0)
    {
        asset refund_asset = amount_eos_sent;
        refund_asset.amount = refund_amount;
        nftcontrak::inline_transfereos(get_self(), owner_account, refund_asset, "Sending refund from the Golden NFT minting.");
    }
}
