/* Demond Yeld Farms HPP */

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/print.hpp>
#include <eosio/system.hpp>

using namespace std;
using namespace eosio;

class [[eosio::contract("dmdfarms")]] dmdfarms:public eosio::contract 
{
    private:
    const symbol dmd_symbol;

    struct [[eosio::table]] stakes
    {
        name owner_account;

        uint64_t total_stake_quantity;
        uint32_t release_date;

        uint64_t primary_key()const { return owner_account.value; } 
    };
    typedef eosio::multi_index< "staketable"_n, stakes  > staketable;


    struct [[eosio::table]] total_staked
    {
        name     key;

        bool     locked;

        asset    dmd_total_staked_30d;
        asset    dmd_total_staked_60d;
        asset    dmd_total_staked_90d;

        uint32_t last_reward_time;

        uint32_t dmd_issue_frequency_30d;
        uint32_t dmd_issue_frequency_60d;
        uint32_t dmd_issue_frequency_90d;
        
        uint64_t dmd_qty_remaining_30d;
        uint64_t dmd_qty_remaining_60d;
        uint64_t dmd_qty_remaining_90d;

        uint64_t primary_key()const { return key.value; } 
    };
    
    typedef eosio::multi_index< "totaltable"_n, total_staked > totaltable;

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
            eosio::name("dmd.efi"), /* Token Contrak */
            eosio::name("transfer"),
            transfer{from, to, quantity, memo});
            transfer_action.send();
    }

    public:
    using contract::contract;

    [[eosio::action]]
    void registerstake(const name& owner_account, const name& to, const asset& stake_quantity_dmd, std::string memo);

    dmdvault(name receiver, name code, datastream<const char *> ds):contract(receiver, code, ds), dmd_symbol("DMD", 4), {}
};
