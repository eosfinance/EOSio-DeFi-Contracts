#include <dmdvault.hpp>


/* The Demond Vault */

/* Users send Demond and get more Demond */

/* 30 days - 60 days and 90 days staking */

/* Functions just like the Dolphin Staking feature */

/* User's Demonds Timestamp will reset every time he deposits for any of the three options - 30 days - 60 or 90 days */

/* He will have three separate stakes, and every time he adds anything to the specific stake, it will reset his release_timestamp and add it to the total */

/* Should we also make it that the users receive HUB & DOP when they stake in the DMD Vault */



void stake::setlocked(bool locked)
{ /* Need to unlock the vault after deploying and setting the tables */
    require_auth(get_self());

    totaltable total_staked(get_self(), "totals"_n.value); 
    auto total_it = total_staked.find("totals"_n.value);
    if(total_it != total_staked.end()) 
    {
        uint32_t now = current_time_point().sec_since_epoch();
        total_staked.modify(total_it, get_self(), [&]( auto& row ) 
        {
            row.locked = locked;
            row.last_reward_time = now;
        });
    } 
    else 
        return;
}
/* Setter function that needs to be called after the contract is deployed to initialize the totals table. */
void stake::set(

const asset& dmd_total_staked_30d, 
const asset& dmd_total_staked_60d, 
const asset& dmd_total_staked_90d, 

uint32_t dmd_issue_frequency_30d, 
uint32_t dmd_issue_frequency_60d, 
uint32_t dmd_issue_frequency_90d,
        
uint64_t dmd_qty_remaining_30d, 
uint64_t dmd_qty_remaining_60d, 
uint64_t dmd_qty_remaining_90d )
{
    require_auth(get_self());
    totaltable total_staked(get_self(), "totals"_n.value);

    uint32_t now = current_time_point().sec_since_epoch();

    auto total_it = total_staked.find("totals"_n.value);
    if(total_it == total_staked.end()) 
    {
        total_staked.emplace(get_self(), [&](auto& row) 
        {   /* Set will start the vault locked. Needs unlocking after setting the tables */
            row.key = "totals";

            row.locked = true;

            row.dmd_total_staked_30d = dmd_total_staked_30d;
            row.dmd_total_staked_60d = dmd_total_staked_60d;
            row.dmd_total_staked_90d = dmd_total_staked_90d;

            row.last_reward_time = now;

            uint32_t dmd_issue_frequency_30d;
            uint32_t dmd_issue_frequency_60d;
            uint32_t dmd_issue_frequency_90d;
        
            uint64_t dmd_qty_remaining_30d;
            uint64_t dmd_qty_remaining_60d;
            uint64_t dmd_qty_remaining_90d;
        });
    } 
    else 
        total_staked.modify(total_it, get_self(),[&]( auto& row) 
        {
            row.dmd_total_staked      = total_staked_dmd;
            /* We'll always set the locked to zero on set(). We have to do two things to initialize the staking */
            row.locked                = 0; 
            /* How many coins are issued per second (will be multiplied by 1000 to fit the precision) */
            row.dmd_issue_frequency   = dmd_issue_frequency;
            row.last_reward_time = now;
        });
}

[[eosio::on_notify("dmd.efi::transfer")]]
void stake::registerstake(const name& owner_account, const name& to, const asset& stake_quantity_dmd, std::string memo)
{
    if (to != get_self() || owner_account == get_self())
    {
        print("error: these are not the droids you are looking for.");
        return;
    }

    if (owner_account == "efi"_n)
        return;  /* This account can always send DMD to the vault contract to top it off */

    /* Need to check that there's still DMD left in the vault to give out */
    /* Need to have a counter for the DMD issued to users */
    /* So we will have a totaltable here too, where we will keep track of total_dmd_staked in each period, and dmd_remaining_qty for each period. */

    check(stake_quantity_dmd.amount >= 10000, "error: must stake a minimum of 1 DMD.");
    check(stake_quantity_dmd.symbol == dmd_symbol, "error: these are not the droids you are looking for.");

    /* Need to check that the memo is either "30", "60" or "90". Otherwise we block the transfer. */
    bool validmemo = false;
    if ( (memo == "30") || (memo == "60") || (memo == "90") )
        bool validmemo = true;

    check(validmemo == true, "error: must specify staking period: 30, 60 or 90.");

    /* Now we need to convert the string into uint memo_integer_value. We can use the uint for the pool scope as well */

    memo_integer_value = uint8_t(memo); /* Test this */

    uint32_t now = current_time_point().sec_since_epoch();
    uint32_t seconds_in_a_month = 2629743;
    /* Open the staking table with the appropiate scope */
    staketable stakes(get_self(), memo_integer_value);
    auto stakers_it = stakes.find(owner_account.value);
    if (stakers_it == stakes.end())
    {/* First time depositing user */
        stakes.emplace.emplace(owner_account, [&](auto& row)
        {
            row.owner_account = owner_account;        
            row.total_stake_quantity = stake_quantity_dmd.amount;
            row.release_date = now*seconds_in_a_month*memo_integer_value;
        });
    }
    /* We also have to check to see if locked == false, and if so, we won't allow anyone to send EOS */
    bool locked = total_it->locked;
    check(locked == true, "error: the DMD vault is currently locked.");

    /* Update the total staked variable in the table */
    total_staked.modify(total_it, get_self(), [&](auto& row) 
    {
        row.dmd_total_staked += stake_quantity_dmd;
    });

    staketable staked(get_self(), get_self().value);

    auto staked_it = staked.find(owner_account.value);
    if(staked_it == staked.end())
    { /* First time depositing anything into staking */
        staked.emplace(get_self(), [&](auto& row)
        {
            row.owner_account = owner_account;
            row.dmd_staked_amount = stake_quantity_dmd;
            row.dmd_unclaimed_amount = stake_quantity_dmd;
            row.dmd_claimed_amount = stake_quantity_dmd;
            row.dmd_unclaimed_amount.amount = 0;
            row.dmd_claimed_amount.amount = 0;
        });
    }
    else if (staked_it->dmd_staked_amount.amount == 0)
    { /* First time depositing DMD, but he has deposited another type of coin before */
        staked.modify(staked_it, get_self(), [&](auto& row) 
        {   
            row.dmd_staked_amount = stake_quantity_dmd;
            row.dmd_unclaimed_amount = stake_quantity_dmd;
            row.dmd_claimed_amount = stake_quantity_dmd;
            row.dmd_unclaimed_amount.amount = 0;
            row.dmd_claimed_amount.amount = 0;
        });
    }
    else
    { /* It's not his first time depositing this type of coin */
        staked.modify(staked_it, get_self(), [&](auto& row) 
        {   
            row.dmd_staked_amount.amount += stake_quantity_dmd.amount;
        });
    }
}
