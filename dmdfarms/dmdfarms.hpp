/* Demond Yeld Farms HPP */

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/print.hpp>
#include <eosio/system.hpp>

using namespace std;
using namespace eosio;

class [[eosio::contract("efimine")]] efimine:public eosio::contract 
{
    private:
    const symbol dop_box_lp_symbol;
    const symbol hub_box_lp_symbol;
    const symbol dmd_box_lp_symbol;

    const symbol dop_symbol;
    const symbol hub_symbol;
    const symbol dmd_symbol;


    struct [[eosio::table]] accounts
    {
        asset    box_lp_tokens;

        uint64_t primary_key()const { return box_lp_tokens.symbol.code().raw(); }
    };
    typedef eosio::multi_index< "boxtable"_n, accounts > boxtable;


    struct [[eosio::table]] registered_accounts
    {
        name   owner_account;

        uint64_t  hub_snapshot_lp_amount;
        uint64_t  dop_snapshot_lp_amount;
        uint64_t  dmd_snapshot_lp_amount;

        uint64_t  hub_before_lp_amount;
        uint64_t  dop_before_lp_amount;
        uint64_t  dmd_before_lp_amount;

        uint64_t  dop_claimed_amount;
        uint64_t  hub_claimed_amount;
        uint64_t  dmd_claimed_amount;

        uint64_t  dop_unclaimed_amount;
        uint64_t  hub_unclaimed_amount;
        uint64_t  dmd_unclaimed_amount;

        uint64_t primary_key()const { return owner_account.value; }
    };
    typedef eosio::multi_index< "lptable"_n, registered_accounts > lptable;


    struct [[eosio::table]] total_lpstats 
    {
        name     key;

        bool     locked;

        uint32_t mining_start_time;
        uint32_t halving1_deadline;
        uint32_t halving2_deadline;
        uint32_t halving3_deadline;
        uint32_t halving4_deadline;
        uint32_t last_reward_time;

        uint32_t hub_issue_frequency = hub_issue_frequency;
        uint32_t dop_issue_frequency = dop_issue_frequency;
        uint32_t dmd_issue_frequency = dmd_issue_frequency;

        uint64_t primary_key()const { return key.value; } 
    };
    typedef eosio::multi_index< "totaltable"_n, total_lpstats > totaltable;


    void inline_transferhub(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo) const
    {
        struct transfer
        {
            eosio::name from;
            eosio::name to;
            eosio::asset quantity;
            std::string memo;
        };

        eosio::action transfer_action = eosio::action(
            eosio::permission_level(get_self(), "active"_n),
            eosio::name("hub.efi"), // name of the contract
            eosio::name("transfer"),
            transfer{from, to, quantity, memo});
            transfer_action.send();
    }

    void inline_transferdop(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo) const
    {
        struct transfer
        {
            eosio::name from;
            eosio::name to;
            eosio::asset quantity;
            std::string memo;
        };

        eosio::action transfer_action = eosio::action(
            eosio::permission_level(get_self(), "active"_n),
            eosio::name("dop.efi"), // name of the contract
            eosio::name("transfer"),
            transfer{from, to, quantity, memo});
            transfer_action.send();
    }

    void inline_transferdmd(eosio::name from, eosio::name to, eosio::asset quantity, std::string memo) const
    {
        struct transfer
        {
            eosio::name from;
            eosio::name to;
            eosio::asset quantity;
            std::string memo;
        };

        eosio::action transfer_action = eosio::action(
            eosio::permission_level(get_self(), "active"_n),
            eosio::name("dmd.efi"), // name of the contract
            eosio::name("transfer"),
            transfer{from, to, quantity, memo});
            transfer_action.send();
    }

    asset get_asset_amount(name owner_account, asset lptoken)
    {/* user account name and glue dog LP tokan, amount doesnt matter just put something there to quickly have the symbol */
        symbol lpsymbol = lptoken.symbol;
        asset lpbalance;
        lpbalance.amount = 0;

        accounts to_acnts( name{"lptoken.defi"}, owner_account.value );
        for (auto box_it = to_acnts.begin(); box_it != to_acnts.end(); box_it++)
        {
            if((box_it->balance).symbol == lpsymbol)
            {
                  lpbalance = box_it->balance;
                  break;
            }
        }
        return lpbalance;
    }

    public:
    using contract::contract;

    [[eosio::action]]
    void set(uint32_t hub_issue_frequency, uint32_t dop_issue_frequency, uint32_t dmd_issue_frequency , bool locked);
    [[eosio::action]]
    void setlocked(bool locked);
    [[eosio::action]]
    void registeruser(const name& owner_account);
    [[eosio::action]]
    void claimrewards(const name& owner_account);
    [[eosio::action]]
    void issue();

    efimine(name receiver, name code, datastream<const char *> ds):contract(receiver, code, ds), hub_symbol("HUB", 4), dop_symbol("DOP", 4), dmd_symbol("DMD", 4),
                                                            hub_box_lp_symbol;("BOXBMZ", 0), dop_box_lp_symbol;("BOXBMY", 0), dmd_box_lp_symbol;("BOXBMU", 0) {}
};
