Ext.define('TableApparatusApp.store.DocumentListStore', {
    extend: 'Ext.data.Store',

    requires: [
        'TableApparatusApp.model.DocumentListModel',
    ],

    constructor: function(cfg) {
        var hashsplit = document.location.href.split('#');
        var slashsplit = hashsplit[1].split('/');
        var mvd = slashsplit[0];
        var crit_ed = slashsplit[1];
        var me = this;
        cfg = cfg || {};
        console.log('/sites/all/modules/austese_repository/api/mvds/');
        console.log('/islandora/collation/islandora%3A1137/Blarger/list');
        var project = jQuery('#metadata').data('project');
        me.callParent([Ext.apply({
            storeId: 'DocumentListStore',
            autoLoad: true,
            model: 'TableApparatusApp.model.DocumentListModel',
            proxy: {
                type: 'ajax',
                url: '/islandora/collation/' + crit_ed + '/' + mvd + '/list',
                //url: '/sites/all/modules/austese_repository/api/mvds/' + (project? "?project="+project:""),
                reader: {
                    type: 'json',
                    root: 'results'
                }
            }
        }, cfg)]);
    }
});