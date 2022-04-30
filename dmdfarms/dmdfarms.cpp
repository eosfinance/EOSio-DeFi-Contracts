#include <dmdfarms.hpp>
/*

The Demond Yeld Farms.

TODO:

Should have setpool set the pools always with "is_active = 0". 
And then we have another function: activatepool(), and when that is called, the halvings and mining start time are also automatically set.

1. DMD Vault where users stake DMD for 3-6-9 months and get DMD rewards.

2. The NFTs with +10% bonus.

DONE:

3. Yield Farms. 

*/
void dmdfarms::init()
{   /* Need to call this first after deploying the contract. This function will set "last_pool_id". */
    require_auth(get_self());
    globaltable globals(get_self(), "global"_n.value);
    auto global_it = globals.find("global"_n.value);
    if (global_it == globals.end())
    {
        globals.emplace(get_self(), [&](auto& row) 
        {
            row.key = "global"_n;
            row.last_pool_id = 0;
        });
    }
    else
    {   /* Counts pools and set the last_pool_id automatically. Must not have non-consecutive pools. */
        uint16_t i = 0;
        while (i <= 32) /* Could we possibly ever have more than 32 pools? */
        {   
            pooltable pool_stats(get_self(),i);
            auto pool_it = pool_stats.find(i);
            if (pool_it != pool_stats.end())
            {
                i++;
                continue;
            }
            globals.modify(global_it, get_self(),[&]( auto& row) 
                {   row.last_pool_id = i-1   ;});
            break;
        }
    }
}

void dmdfarms::setpool(uint16_t pool_id, uint32_t dmd_issue_frequency, bool is_active, uint64_t min_lp_tokens, asset box_asset_symbol, string pool_name, uint64_t dmd_mine_qty_remaining)
{
    require_auth(get_self());

    globaltable globals(get_self(), "global"_n.value);
    auto global_it = globals.find("global"_n.value);
    check(global_it != globals.end(),"error: must init global first.");

    pooltable pool_stats(get_self(), pool_id);
    auto pool_it = pool_stats.find(pool_id);

    uint32_t now = current_time_point().sec_since_epoch();
    uint32_t seconds_in_a_month = 2629743;
    uint32_t months_between_halvings = 3;

    if(pool_it == pool_stats.end()) 
    {   /* Some of these rows will only get modified the first time setpool() is ran */
        if (pool_id != 0)
        {   /* Update last_pool_id on emplace. */
            check(pool_id == global_it->last_pool_id+1,"error: pool_id must be consecutive.");
            globals.modify(global_it, get_self(),[&]( auto& row) 
            {   row.last_pool_id += 1   ;});
        }

        pool_stats.emplace(get_self(), [&](auto& row) 
        {
            row.pool_id = pool_id;
            row.is_active = is_active;
            /* Halvings are applied separately to each pool */
            row.mining_start_time = now;
            row.halving1_deadline = now+(1*seconds_in_a_month * months_between_halvings);
            row.halving2_deadline = now+(2*seconds_in_a_month * months_between_halvings);
            row.halving3_deadline = now+(3*seconds_in_a_month * months_between_halvings);
            row.halving4_deadline = now+(4*seconds_in_a_month * months_between_halvings);
            row.last_reward_time = now;

            row.dmd_mine_qty_remaining = dmd_mine_qty_remaining; /* How many DMDs are left to be mined in the pool */
            row.dmd_issue_frequency = dmd_issue_frequency;
            row.minimum_lp_tokens = min_lp_tokens;   /* Minimum LP tokens required to earn yield in the pool. Needs to check for this on issue(). */
                                                     /* Should set the minimum to be around 100-200 EOS liquidity, because we don't want to get spammed */
                                                     /* Should implement automatic user unregistering */
            row.box_asset_symbol = box_asset_symbol; /* The BOX-LP Tokens used to identify the pair for the pool. */
            row.pool_name = pool_name;               /* String identifying the pool name, for display purposes only. */
            row.pool_total_lptokens = 0;             /* This is only used to calculate the APR for the display, as our worker counts them every new cycle */
        });

        /* Here we'll need a loop that always recounts all the pools and keeps a last_pool_id variable */
        /* We should also have a check in place that will not allow you to add non-consecutive pool_ids */
    }
    else
        pool_stats.modify(pool_it, get_self(),[&]( auto& row) 
        { /* We can always modify the pools with setpool() at any time */
            row.is_active = is_active;

            row.dmd_issue_frequency = dmd_issue_frequency;
            row.dmd_mine_qty_remaining = dmd_mine_qty_remaining; /* How many DMDs are left to be mined in the pool */
            row.minimum_lp_tokens = min_lp_tokens;
            row.box_asset_symbol = box_asset_symbol;
            row.pool_name = pool_name;

            row.last_reward_time = now;
        });
}

void dmdfarms::issue(uint16_t pool_id) /* Should add an offset here to control batching */
{   /* The worker will do an issue for each individual pool. */
    require_auth("worker.efi"_n);

    /* Mining rate calculations */
    pooltable pool_stats(get_self(), pool_id);
    auto pool_it = pool_stats.find(pool_id);
    check(pool_it != pool_stats.end(), "error: pool_id not found.");
    check(pool_it->is_active == true, "error: specified pool is inactive.");

    uint8_t mining_rate_handicap;
    uint32_t now = current_time_point().sec_since_epoch();
    /* Determine halving handicap */
    if (now  < pool_it->halving1_deadline)  {  mining_rate_handicap = 1;  }
    if (now >= pool_it->halving1_deadline)  {  mining_rate_handicap = 2;  }
    if (now >= pool_it->halving2_deadline)  {  mining_rate_handicap = 4;  }
    if (now >= pool_it->halving3_deadline)  {  mining_rate_handicap = 8;  }
    if (now >= pool_it->halving4_deadline)  {  mining_rate_handicap = 16; }

    uint16_t issue_precision  = 10000; // With our current algorithm: if we set ("issue_frequency" == 100): we release 0.01 tokens per second.
                                      //    and if we set ("issue_frequency" == 1):   we release 0.0001 tokens per second.

    /* How many coins are issued every second. Multiplied by 10000 for tokens with precision 4. Divided by "issue_precision" for extra control */
    uint32_t augmented_dmd_issue_frequency = pool_it->dmd_issue_frequency*10000 / issue_precision / mining_rate_handicap;

    /* How many seconds have passed until now and last_reward_time */
    uint32_t seconds_passed = now - pool_it->last_reward_time; 
    eosio::print_f("Seconds passed since last issue(): [%] \n",seconds_passed);

    /* Determine how much total DMD reward should be issued in this transaction/cycle/block */
    uint64_t total_dmd_released = seconds_passed * augmented_dmd_issue_frequency;
    /* Check if there's enough DMD left in the pool */
    check(total_dmd_released > pool_it->dmd_mine_qty_remaining,"error: no more DMD left to mine in the pool.");

    lptable registered_accounts(get_self(), pool_id);

    /* Get total registered lptokens for this issue cycle */
    auto current_iteration = registered_accounts.begin();
    auto end_itr = registered_accounts.end();

    uint64_t pool_total_lptokens = 0; /* Counting the total LP Tokens of everyone currently mining in pool ("pool_id") */

    while (current_iteration != end_itr)
    {/* Count everyone's box lptokens */
        pool_total_lptokens += get_asset_amount(current_iteration->owner_account, pool_it->box_asset_symbol).amount;
        ++current_iteration; }

    eosio::print_f("Finished counting total lptokens for this issue cycle.\n");
    eosio::print_f("Pool_total_lptokens: [%]\n",pool_total_lptokens);
    /* Record the pool_total_lptokens in the pool_id table. */
    pool_stats.modify(pool_it, get_self(),[&]( auto& row) 
    {  row.pool_total_lptokens = pool_total_lptokens;  });
    /* Reset the counter */
    current_iteration = registered_accounts.begin();
    /* Recount exactly how much DMD was released this cycle. This will also include the NFT bonus as well */
    uint64_t precise_total_dmd_released = 0;

    /* Loop again and give every user their proper rewards */
    while (current_iteration != end_itr)
    {
        name current_owner = current_iteration->owner_account;
        eosio::print_f("Checking lptoken information for: [%]\n",current_iteration->owner_account);
        /* Check the Defibox LP tables to see how many LPTokens each user has */
        asset user_box_lptoken = get_asset_amount(current_iteration->owner_account, pool_it->box_asset_symbol);
        /* Must definitely test these. */
        uint64_t boxlptoken_current_snap    = user_box_lptoken.amount;                       /* Current snapshot is what we have in this cycle */
        uint64_t boxlptoken_previous_snap   = current_iteration->boxlptoken_snapshot_amount; /* This is the user's balance in the previous turn */
        uint64_t boxlptoken_before_snapshot = current_iteration->boxlptoken_before_amount;   /* This is the user's balance two turns before current turn */

        /* We'll use these to calculate the user's lp rewards and add to his unclaimed_balance */
        uint64_t actual_box_lp_calculation_amount; /* We'll use whatever is lower between current_snapshot and before_snapshot. */
        if (boxlptoken_current_snap > current_iteration->boxlptoken_before_amount) { actual_box_lp_calculation_amount = boxlptoken_before_snapshot; }
                                                                             else  { actual_box_lp_calculation_amount = boxlptoken_current_snap; }
        uint64_t dmd_unclaimed_amount = 0;
        /* Calculate the user's unclaimed rewards and add it to the dmd_unclaimed_amount */

        if (actual_box_lp_calculation_amount > 0)
        {   float dmd_percentage = float(actual_box_lp_calculation_amount)/float(pool_total_lptokens) * 100; 
            dmd_unclaimed_amount = (dmd_percentage*total_dmd_released)/0.01/10000;  } /* (divided by 10000) for coins with precision 4 */
            /* Here we should see if they have NFTs or not. If they do, we can increase the unclaimed_amount by 5-10% */

        /* Modify the table and have before = snapshot and snapshot = current_snapshot */
        /* Add the user's unclaimed rewards if necessary */
        registered_accounts.modify(current_iteration, get_self(), [&](auto& row)
        {
            row.boxlptoken_before_amount   = boxlptoken_previous_snap;  /* Must definitely test this. */
            row.boxlptoken_snapshot_amount = boxlptoken_current_snap;   /* Must definitely test this. */
            row.dmd_unclaimed_amount      += dmd_unclaimed_amount;      /* Must definitely test this. */
        });

        precise_total_dmd_released += dmd_unclaimed_amount;

        ++ current_iteration;
    }/* The NFTs calculation should be simple. We will check to see if user has or has not got NFT, and we will add a +10% to his farming bonus at the end. */
    /* Modify the dmd_mine_qty_remaining row from the pools table. */
    pool_stats.modify(pool_it, get_self(),[&]( auto& row) 
    {  row.dmd_mine_qty_remaining -= precise_total_dmd_released;
       row.last_reward_time = now;  });
}

void dmdfarms::registeruser(const name& owner_account, uint16_t pool_id)
{   /* User pays for their own RAM to be added to the table. Users will have to register to each pool separately. */
    /* Function will check if the user has a minimum amount of LP tokens, and if not, they will not be registered for the pool. */
    require_auth(owner_account);

    /* Check pool integrity */
    pooltable pool_stats(get_self(), pool_id);
    auto pool_it = pool_stats.find(pool_id);
    check(pool_it != pool_stats.end(), "error: pool_id not found.");
    check(pool_it->is_active == true, "error: specified pool is inactive.");

    /* Check that the user has not already registered */
    lptable registered_accounts(get_self(), pool_id);
    auto registered_it = registered_accounts.find(owner_account.value);
    check(registered_it == registered_accounts.end(), "error: User has already been registered for the pool.");

    /* Check if user has the minimum amount of lptokens needed */
    asset pool_lptokens = get_asset_amount(owner_account, pool_it->box_asset_symbol);
    eosio::print_f("Checking LP Token quantity: [%] for [%]\n",pool_it->box_asset_symbol, owner_account);
    eosio::print_f("User has [%] pool_lptokens\n",pool_lptokens);
    check(pool_lptokens.amount >= pool_it->minimum_lp_tokens, "User does not meet the minumum LP size requirement for the specific pool. Please add more liquidity.");
    /* Add the user in the table at this point */
    registered_accounts.emplace(owner_account, [&](auto& row)
    {
        /* Here we'll have the `before` and the `snapshot` LP amount counter. */
        /* This is the first time they are added, so most variables are set to zero. */
        row.owner_account = owner_account;
        row.boxlptoken_snapshot_amount = pool_lptokens.amount;
        row.boxlptoken_before_amount = 0;
        row.dmd_claimed_amount = 0;
        row.dmd_unclaimed_amount = 0;
    });
}

void dmdfarms::claimrewards(const name& owner_account, uint16_t pool_id)
{   
    require_auth(owner_account);

    /* Check pool integrity */
    pooltable pool_stats(get_self(), pool_id);
    auto pool_it = pool_stats.find(pool_id);
    check(pool_it != pool_stats.end(), "error: Specified mining pool is not valid.");

    /* Check if user is registered and if he has a claimable balance */
    lptable registered_accounts(get_self(), pool_id);
    auto lprewards_it = registered_accounts.find(owner_account.value);
    check(lprewards_it != registered_accounts.end(), "error: User is not registered.");
    check(lprewards_it->dmd_unclaimed_amount > 0, "error: No rewards available to claim.");

    /* Check the registered user table, update the claimed_amount and unclaimed_amount */
    if (lprewards_it->dmd_unclaimed_amount > 0)
    {
        asset dmd_reward_amount;
        dmd_reward_amount.symbol = dmd_symbol;
        dmd_reward_amount.amount = lprewards_it->dmd_unclaimed_amount;

        registered_accounts.modify(lprewards_it, owner_account, [&](auto& row)
        {
            row.dmd_claimed_amount += dmd_reward_amount.amount;
            row.dmd_unclaimed_amount = 0;
        });
        /* Send rewards */
        dmdfarms::inline_transferdmd(get_self(), owner_account, dmd_reward_amount, "I'm LP mining DMD in the Yield Farms !");
    }
}

void dmdfarms::dellastpool()
{   require_auth(get_self());
    /* Get last_pool_id */
    globaltable globals(get_self(), "global"_n.value);
    auto global_it = globals.find("global"_n.value);
    check(global_it != globals.end(),"error: must init global first.");

    pooltable pool_stats(get_self(), global_it->last_pool_id);
    auto itr = pool_stats.begin();

    while (itr != pool_stats.end())
        itr = pool_stats.erase(itr);

    globals.modify(global_it, get_self(),[&]( auto& row) 
            {   row.last_pool_id -= 1   ;});
}

/* DEBUG FUNCTIONS. Should never be used in production. */
void dmdfarms::clearusers(uint16_t pool_id)
{   require_auth(get_self());

    lptable registered_accounts(get_self(), pool_id);
    auto itr = registered_accounts.begin();

    while (itr != registered_accounts.end())
        itr = registered_accounts.erase(itr);
}

void dmdfarms::clearpool(uint16_t pool_id)
{   require_auth(get_self());

    pooltable pool_stats(get_self(), pool_id);
    auto itr = pool_stats.begin();

    while (itr != pool_stats.end())
        itr = pool_stats.erase(itr);
}
