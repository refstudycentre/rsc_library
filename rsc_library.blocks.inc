<?php


/**
 * Implements hook_block_info().
 */
function rsc_library_block_info() {

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
  
  module_load_include('inc','rsc_library','rsc_library.common');
  $lids = rsc_library_get_lids();
  foreach ($lids as $lid) {  // for each library
    $library = node_load($lid);
    
    $blocks["lib_teaser_$lid"] = array(
      'info'  => t("RSC Library: Teaser block for library \"{$library->title}\""),
      'cache' => DRUPAL_CACHE_PER_ROLE,  // not every library is public
    );
  }

  return $blocks;
}


/**
 * Implements hook_block_view().
 * @param string $delta The module-specific identifier of the block as specified
 * in rsc_library_block_info
 * @return array Block array with keys 'subject' and 'content'.
 */
function rsc_library_block_view($delta = '') {

  $block = array();
  
  if (substr($delta,0,11)=="lib_teaser_") {
    
    $lid = substr($delta,11); // remove first 11 characters, the rest is the lid
    if (is_numeric($lid)) {
      $block = array(
        'subject' => '',
        'content' => node_view(node_load($lid), 'teaser'),
      );
    }
    
  } else if (!empty($GLOBALS['rsc_library']['lid'])) {
    
    $lid = $GLOBALS['rsc_library']['lid'];
    $block_view_function = "rsc_library_block_view_${delta}";
    $block = $block_view_function($lid);

  }

  return $block;
}


/**
 * Generate the content for the featured nodes block
 * function name follows the pattern rsc_library_block_view_$delta
 * @param int $lid Library ID to filter the nodes
 * @return array Block array of the form usually returned by hook_block_view
 */
function rsc_library_block_view_feat_node($lid) {

  $block = array();

  $items = cache_get('rsc_library_feat_nodes');
  if (empty($items)) { // if the data is not available from cache
    $items = array();

    // get a list of links to all promoted nodes in the library:
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', array('rsc_library_article','rsc_library_audio'), 'IN')
      ->propertyCondition('status', 1)
      ->propertyCondition('promote', 1)
      ->addMetaData('account', user_load(1)); // Run the query as user 1 (TODO: remove once access checks are implemented)
    $result = $query->execute();

    if (isset($result['node'])) {
      foreach($result['node'] as $result) {
        $node = node_load($result->nid);

        // determine the library to which this node belongs
        foreach($node as $key => $val) {
          if (substr($key, 0, 14) === 'rscl_category_' && field_get_items('node',$node,$key)) { // if the node is listed under a specific library's categories
            $node_lid = variable_get($key,NULL);

            // group nodes by lid
            // $items[$node_lid][$node->nid] = l($node->title,"node/{$node->nid}"); // TODO: make this configurable: simple link vs. rendered content
            $renderable_node = node_view($node,'rscl_block');
            $items[$node_lid][$node->nid] = render($renderable_node);
          }
        }
      }
    }

    // cache the list of promoted nodes for at least two days
    cache_set('rsc_library_feat_nodes', $items, 'cache', REQUEST_TIME + 2*24*60*60);

  } else { // data is available from cache
    $items = $items->data;
  }

  if (!empty($items[$lid])) {
    $items = $items[$lid]; // just display the current library's nodes

    if (count($items) > 5) {
      // select N random values from the array:
      $items_subset = array_rand($items,5);
      $list_items = array();
      foreach($items_subset as $nid) {
        $list_items[$nid] = $items[$nid];
      }
    } else {
      $list_items = $items;
    }
    shuffle($list_items);

    $block = array(
      'subject' => t('Featured content'),
      'content' => array(
        '#theme' => 'item_list',
        '#items' => $list_items,
      ),
    );

  }

  return $block;

}


/**
 * Generate the content for the new nodes block
 * function name follows the pattern rsc_library_block_view_$delta
 * @param int $lid Library ID to filter the nodes
 * @return array Block array of the form usually returned by hook_block_view
 */
function rsc_library_block_view_new_node($lid) {

  $block = array();

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
      ->addMetaData('account', user_load(1)); // Run the query as user 1 (TODO: remove once access checks are implemented)
    $result = $query->execute();

    if (isset($result['node'])) {
      foreach($result['node'] as $result) {
        $node = node_load($result->nid);

        // determine the library to which this node belongs
        foreach($node as $key => $val) {
          if (substr($key, 0, 14) === 'rscl_category_' && field_get_items('node',$node,$key)) { // if the node is listed under a specific library's categories
            $node_lid = variable_get($key,NULL);

            // group nodes by lid
            $items[$node_lid][$node->nid] = l($node->title,"node/{$node->nid}"); // TODO: make this configurable: simple link vs. rendered content
            // $items[$node_lid][$node->nid] = render(node_view($node,'rscl_block'));
          }
        }
      }
    }

    // cache the list of newest nodes 'permanently', clear during hook_node_save
    cache_set('rsc_library_new_nodes', $items, 'cache', CACHE_PERMANENT);
    // TODO: cache the entire markup, because the order never changes between cache clears

  } else { // data is available from cache
    $items = $items->data;
  }

  if (!empty($items[$lid])) {
    $items = $items[$lid]; // just display the current library's nodes

    $block = array(
      'subject' => t('Newest content'),
      'content' => array(
        '#theme' => 'item_list',
        '#items' => $items,
      ),
    );

  }

  return $block;

}


/**
 * Generate the content for the new nodes block
 * function name follows the pattern rsc_library_block_view_$delta
 * @param int $lid Library ID to filter the nodes
 * @return array Block array of the form usually returned by hook_block_view
 */
function rsc_library_block_view_feat_cat($lid) {

  $prefix = 'rscl';
  $block = array();

  $items = cache_get("rsc_library_feat_cats_{$lid}");
  if (empty($items)) { // if the data is not available from cache
    $items = array();

    $settings = variable_get("rsc_library_{$lid}");
    $vid = $settings['vocabs']['category'];

    // get a list of links to all featured categories in the library:
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'taxonomy_term')
      ->propertyCondition('vid', $vid)
      ->fieldCondition("{$prefix}_featured",'value',1,'=')
      ->addMetaData('account', user_load(1)); // Run the query as user 1 (TODO: remove once access checks are implemented)
    $result = $query->execute();

    if (isset($result['taxonomy_term'])) {
      foreach($result['taxonomy_term'] as $result) {
        $term = taxonomy_term_load($result->tid);
        $items[$term->tid] = l($term->name,"taxonomy/term/{$term->tid}");
      }
    }

    // cache the list of featured categories for at least six days
    cache_set("rsc_library_feat_cats_{$lid}", $items, 'cache', REQUEST_TIME + 6*24*60*60);

  } else { // data is available from cache
    $items = $items->data;
  }

  if (!empty($items)) {

    if (count($items) > 5) {
      // select N random values from the array:
      $items_subset = array_rand($items,5);
      $list_items = array();
      foreach($items_subset as $nid) {
        $list_items[$nid] = $items[$nid];
      }
    } else {
      $list_items = $items;
    }
    shuffle($list_items);

    $block = array(
      'subject' => t('Featured categories'),
      'content' => array(
        '#theme' => 'item_list',
        '#items' => $list_items,
      ),
    );

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


