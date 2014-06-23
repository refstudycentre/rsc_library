<?php


/**
 * Implements hook_block_info().
 */
function rsc_library_block_info() {
  $lid = 1; // TODO: multiple libraries providing multiple blocks

  $blocks['feat_node'] = array(
    'info'  => t('RSC Library: Featured nodes'),
    'cache' => DRUPAL_NO_CACHE,  // we cache the query result inside block_view
  );

  $blocks['new_node'] = array(
    'info'  => t('RSC Library: New nodes'),
    'cache' => DRUPAL_NO_CACHE,  // we cache the query result inside block_view
  );

  $blocks['feat_cat'] = array(
    'info'  => t('RSC Library: Featured categories'),
    'cache' => DRUPAL_NO_CACHE,  // we cache the query result inside block_view
  );

  return $blocks;
}


/**
 * Implements hook_block_view().
 */
function rsc_library_block_view($delta = '') {
  $lid = 1; // TODO: multiple libraries providing multiple blocks
  $prefix = 'rscl';
  $block = array();

  switch ($delta) {
    case 'feat_node':

      $items = cache_get('rsc_library_feat_nodes');
      if (empty($items)) { // if the data is not available from cache
        $items = array();

        // get a list of links to all promoted nodes in the library:
        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', 'node')
          ->entityCondition('bundle', array('rsc_library_article','rsc_library_audio'), 'IN')
          ->propertyCondition('status', 1)
          ->propertyCondition('promote', 1)
          // TODO: select from one of many libraries. no field for that yet
          ->addMetaData('account', user_load(1)); // Run the query as user 1 (TODO: remove once access checks are implemented)
        $result = $query->execute();

        if (isset($result['node'])) {
          foreach($result['node'] as $result) {
            $node = node_load($result->nid);
            $items[$node->nid] = l($node->title,"node/{$node->nid}");
          }
        }

        // cache the list of promoted nodes for at least two days
        cache_set('rsc_library_feat_nodes', $items, 'cache', REQUEST_TIME + 2*24*60*60);

      } else { // data is available from cache
        $items = $items->data;
      }

      if (!empty($items)) {

        // select N random values from the array:
        $items_subset = array_rand($items,5);
        shuffle($items_subset);
        $list_items = array();
        foreach($items_subset as $nid) {
          $list_items[$nid] = $items[$nid];
        }

        $block = array(
          'subject' => t('Featured content'),
          'content' => array(
            '#theme' => 'item_list',
            '#items' => $list_items,
          ),
        );

      }

      break;
    case 'new_node':

      $items = cache_get('rsc_library_new_nodes');
      if (empty($items)) { // if the data is not available from cache
        $items = array();

        // get a list of links to all promoted nodes in the library:
        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', 'node')
          ->entityCondition('bundle', array('rsc_library_article','rsc_library_audio'), 'IN')
          ->propertyCondition('status', 1)
          ->propertyOrderBy('created','DESC')
          ->range(0,5)
          // TODO: select from one of many libraries. no field for that yet
          ->addMetaData('account', user_load(1)); // Run the query as user 1 (TODO: remove once access checks are implemented)
        $result = $query->execute();

        if (isset($result['node'])) {
          foreach($result['node'] as $result) {
            $node = node_load($result->nid);
            $items[$node->nid] = l($node->title,"node/{$node->nid}");
          }
        }

        // cache the list of newest nodes 'permanently', clear during hook_node_save
        cache_set('rsc_library_new_nodes', $items, 'cache', CACHE_PERMANENT);

      } else { // data is available from cache
        $items = $items->data;
      }

      if (!empty($items)) {

        $block = array(
          'subject' => t('Newest content'),
          'content' => array(
            '#theme' => 'item_list',
            '#items' => $items,
          ),
        );

      }

      break;
    case 'feat_cat':

      $items = cache_get('rsc_library_feat_cats');
      if (empty($items)) { // if the data is not available from cache
        $items = array();

        $settings = variable_get('rsc_library');
        $vid = $settings[$lid]['vocabs']['category'];

        // get a list of links to all featured categories in the library:
        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', 'taxonomy_term')
          ->propertyCondition('vid', $vid)
          ->fieldCondition("{$prefix}_featured",'value',1,'=')
          // TODO: select from one of many libraries. no field for that yet
          ->addMetaData('account', user_load(1)); // Run the query as user 1 (TODO: remove once access checks are implemented)
        $result = $query->execute();

        if (isset($result['taxonomy_term'])) {
          foreach($result['taxonomy_term'] as $result) {
            $term = taxonomy_term_load($result->tid);
            $items[$term->tid] = l($term->name,"taxonomy/term/{$term->tid}");
          }
        }

        // cache the list of featured categories for at least six days
        cache_set('rsc_library_feat_cats', $items, 'cache', REQUEST_TIME + 6*24*60*60);

      } else { // data is available from cache
        $items = $items->data;
      }

      if (!empty($items)) {
        
        if (count($items) > 5) {
          // select N random values from the array:
          $items_subset = array_rand($items,5);
          shuffle($items_subset);
          $list_items = array();
          foreach($items_subset as $nid) {
            $list_items[$nid] = $items[$nid];
          }
        } else {
          $list_items = $items;
        }

        $block = array(
          'subject' => t('Featured categories'),
          'content' => array(
            '#theme' => 'item_list',
            '#items' => $list_items,
          ),
        );

      }

      break;
  }

  return $block;
}


/**
 * Implements hook_node_update().
 */
function rsc_library_node_update($node) {

  // the "new_nodes" block's content should be rebuilt
  cache_clear_all('rsc_library_new_nodes','cache');

}


